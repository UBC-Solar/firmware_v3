// can_ingest.cpp
#include <iostream>
#include <vector>
#include <string>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <thread>
#include <cerrno>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <curl/curl.h>

// ---------------- CONFIG ----------------

// Serial
static const char* SERIAL_PORT  = "/dev/ttyUSB0";
static const int   BAUDRATE     = 230400;
static const size_t CHUNK_SIZE  = 16384;

// InfluxDB 2.x
// NOTE: adjust org, bucket, token, and URL to match your setup.
static const char* INFLUX_URL   =
    "http://100.120.214.69/api/v2/write"
    "?org=UBC%20Solar&bucket=CAN_test&precision=ns";

static const char* INFLUX_TOKEN = "token";

// Time source for Influx _time
static const bool  USE_NOW_TIME = false;   // false = use CAN timestamp, true = use now()

// Frame format: 21 bytes per record encoded as 42 ASCII hex chars on one line.
static const int FRAME_LEN_BYTES = 21;
static const int FRAME_HEX_LEN   = 42;

// Batching
static const size_t MAX_LINES_PER_BATCH = 5000;  // max frames per HTTP POST
static const double FLUSH_INTERVAL_S    = 0.5;   // flush at least twice per second

// ---------------- GLOBALS ----------------

int   serial_fd   = -1;
CURL* curl_handle = nullptr;
bool  running     = true;

// simple stats
uint64_t frames_seen     = 0;
uint64_t frames_parsed   = 0;
uint64_t frames_sent     = 0;
uint64_t parse_errors    = 0;
uint64_t http_failures   = 0;

// ---------------- UTILS ----------------

// map hex char -> nibble (0-15), or -1 if invalid
int hex_char_to_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

// convert exactly 2 hex chars -> byte, return success
bool hex2byte(char hi, char lo, uint8_t& out) {
    int nh = hex_char_to_nibble(hi);
    int nl = hex_char_to_nibble(lo);
    if (nh < 0 || nl < 0) return false;
    out = static_cast<uint8_t>((nh << 4) | nl);
    return true;
}

// convert 42 hex chars -> 21 bytes
bool hex42_to_21bytes(const std::string& hex, std::array<uint8_t, FRAME_LEN_BYTES>& out) {
    if (hex.size() < FRAME_HEX_LEN) return false;
    for (int i = 0; i < FRAME_LEN_BYTES; ++i) {
        uint8_t b;
        if (!hex2byte(hex[2 * i], hex[2 * i + 1], b)) {
            return false;
        }
        out[i] = b;
    }
    return true;
}

// ---------------- SERIAL SETUP ----------------

int open_serial(const char* port, int baud) {
    int fd = ::open(port, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open serial");
        return -1;
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        ::close(fd);
        return -1;
    }

    cfmakeraw(&tty);

    speed_t speed;
    switch (baud) {
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        default:     speed = B230400; break; // adjust if you use others
    }

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    // Non-blocking with short timeout
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1; // deciseconds

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        ::close(fd);
        return -1;
    }

    return fd;
}

// ---------------- TIMESTAMP PARSER ----------------

// Interpret first 8 bytes as big-endian double (to match your Python default path).
// If your hardware actually sends ms as uint64, change this accordingly.
double parse_timestamp_seconds(const uint8_t* ts_bytes) {
    // We read as big-endian double
    uint64_t raw = 0;
    for (int i = 0; i < 8; ++i) {
        raw = (raw << 8) | ts_bytes[i];
    }
    double val;
    static_assert(sizeof(double) == sizeof(uint64_t), "double not 64-bit?");
    std::memcpy(&val, &raw, sizeof(double));
    return val;
}

// ---------------- INFLUXDB (LINE PROTOCOL + CURL) ----------------

bool init_curl() {
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (!curl_handle) return false;

    curl_easy_setopt(curl_handle, CURLOPT_URL, INFLUX_URL);
    curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);

    struct curl_slist* headers = nullptr;
    std::string auth = "Authorization: Token " + std::string(INFLUX_TOKEN);
    headers = curl_slist_append(headers, auth.c_str());
    headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

    // Optional: follow redirects, timeouts, etc.
    // curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    return true;
}

bool send_batch_to_influx(const std::string& payload) {
    if (payload.empty()) return true;
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, payload.size());

    CURLcode res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        std::cerr << "Influx write failed: " << curl_easy_strerror(res) << "\n";
        http_failures++;
        return false;
    }
    return true;
}

// Append one frame as a line-protocol entry to 'batch'
void append_frame_line(const std::array<uint8_t, FRAME_LEN_BYTES>& frame,
                       std::string& batch)
{
    // Layout: [0..7] = ts, [8..11] = id, [12..19] = data
    const uint8_t* ts_bytes   = frame.data();
    const uint8_t* id_bytes   = frame.data() + 8;
    const uint8_t* data_bytes = frame.data() + 12;

    double ts_sec = parse_timestamp_seconds(ts_bytes);

    uint32_t can_id = (static_cast<uint32_t>(id_bytes[0]) << 24) |
                      (static_cast<uint32_t>(id_bytes[1]) << 16) |
                      (static_cast<uint32_t>(id_bytes[2]) << 8)  |
                       static_cast<uint32_t>(id_bytes[3]);

    // Build measurement
    // Example: can_frame,source=pi4 can_id=1234i,d0=...,d1=...,can_timestamp=... 1234567890000000000
    batch.reserve(batch.size() + 128);

    batch.append("can_frame,source=pi4"); // measurement + tag(s)
    batch.append(" ");

    // fields
    // can_id as integer
    batch.append("can_id=");
    batch.append(std::to_string(can_id));
    batch.append("i");

    // data bytes as integer fields
    for (int i = 0; i < 8; ++i) {
        batch.append(",d");
        batch.append(std::to_string(i));
        batch.append("=");
        batch.append(std::to_string(static_cast<int>(data_bytes[i])));
        batch.append("i");
    }

    // can_timestamp as float
    batch.append(",can_timestamp=");
    batch.append(std::to_string(ts_sec));

    // raw21 as hex string (string field) – optional but very useful
    batch.append(",raw21=\"");
    char buf[3];
    for (int i = 0; i < FRAME_LEN_BYTES; ++i) {
        std::snprintf(buf, sizeof(buf), "%02X", frame[i]);
        batch.append(buf);
    }
    batch.append("\"");

    // Influx timestamp (ns)
    long long ts_ns;
    if (USE_NOW_TIME) {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        ts_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    } else {
        ts_ns = static_cast<long long>(ts_sec * 1e9);
    }

    batch.append(" ");
    batch.append(std::to_string(ts_ns));
    batch.append("\n");
}

// ---------------- SIGNAL HANDLERS ----------------

void handle_sigint(int) {
    running = false;
}

// ---------------- MAIN LOOP ----------------

int main() {
    std::signal(SIGINT, handle_sigint);
    std::signal(SIGTERM, handle_sigint);

    serial_fd = open_serial(SERIAL_PORT, BAUDRATE);
    if (serial_fd < 0) {
        std::cerr << "Failed to open serial port " << SERIAL_PORT << "\n";
        return 1;
    }

    if (!init_curl()) {
        std::cerr << "Failed to init CURL\n";
        return 1;
    }

    std::string ascii_buf;
    ascii_buf.reserve(65536);

    std::string batch;
    batch.reserve(1024 * 1024);

    auto last_flush = std::chrono::steady_clock::now();
    uint64_t lines_in_batch = 0;

    std::cout << "C++ CAN->Influx ingest started on "
              << SERIAL_PORT << " @ " << BAUDRATE << " baud\n";

    while (running) {
        char tmp[CHUNK_SIZE];
        ssize_t n = ::read(serial_fd, tmp, sizeof(tmp));
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // nothing to read
            } else {
                perror("read");
                break;
            }
        } else if (n > 0) {
            // append to ascii buffer
            ascii_buf.append(tmp, tmp + n);

            // process complete lines (terminated by "\r\n")
            for (;;) {
                std::size_t pos = ascii_buf.find("\r\n");
                if (pos == std::string::npos) break;

                std::string line = ascii_buf.substr(0, pos);
                ascii_buf.erase(0, pos + 2); // remove line + CRLF

                frames_seen++;

                // remove spaces just in case
                std::string hex_line;
                hex_line.reserve(line.size());
                for (char c : line) {
                    if (c != ' ' && c != '\t') {
                        hex_line.push_back(c);
                    }
                }

                if (hex_line.size() < FRAME_HEX_LEN) {
                    parse_errors++;
                    continue;
                }

                // If line has more than 42 chars, we can slide through it in 42-char chunks
                for (std::size_t offset = 0;
                     offset + FRAME_HEX_LEN <= hex_line.size();
                     offset += FRAME_HEX_LEN)
                {
                    std::string frame_hex = hex_line.substr(offset, FRAME_HEX_LEN);
                    std::array<uint8_t, FRAME_LEN_BYTES> frame_bytes{};

                    if (!hex42_to_21bytes(frame_hex, frame_bytes)) {
                        parse_errors++;
                        continue;
                    }

                    frames_parsed++;
                    append_frame_line(frame_bytes, batch);
                    lines_in_batch++;
                    frames_sent++;

                    // flush on batch size
                    if (lines_in_batch >= MAX_LINES_PER_BATCH) {
                        if (!send_batch_to_influx(batch)) {
                            // If it fails, we drop this batch; could be improved to retry.
                        }
                        batch.clear();
                        lines_in_batch = 0;
                        last_flush = std::chrono::steady_clock::now();
                    }
                }
            }
        }

        // time-based flush
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - last_flush).count();
        if (!batch.empty() && dt >= FLUSH_INTERVAL_S) {
            if (!send_batch_to_influx(batch)) {
                // drop batch on failure; could add retry/backoff.
            }
            batch.clear();
            lines_in_batch = 0;
            last_flush = now;
        }

        // small sleep when idle to avoid busy loop
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // final flush
    if (!batch.empty()) {
        send_batch_to_influx(batch);
    }

    std::cout << "Shutting down.\n";
    std::cout << "frames_seen="   << frames_seen
              << " frames_parsed=" << frames_parsed
              << " frames_sent="   << frames_sent
              << " parse_errors="  << parse_errors
              << " http_failures=" << http_failures
              << std::endl;

    if (serial_fd >= 0) ::close(serial_fd);
    if (curl_handle) curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 0;
}
