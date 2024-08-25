# Coding Style
Solar's coding style can be broken down into two parts. 
* Macro styles
    * This standard explains hows to organize your new feature and utilize the concept of software components.
* Micro styles
    * This standard explains when to comment, naming conventions, and any line specific standards.


## Macro Style


## Micro Style
The sections below include
* Naming Convention for Software Components (Modules)
* Naming Convention for Local and Global Variables
* Commenting
* Formatting

### Naming Convention for Software Components (Modules)
As explained in the Macro Style section, we use software components (synonymous with "modules" from here on out) to abstract functionality and enforce portability. While we briefly mentioned how to set up the file structure and workflow of modules, this section explicitly covers the necessary naming conventions when writing these modules. 

*Note: If you are writing code inside a pre-existing component or module then these rules will still apply* 

#### Understanding Your Module

Before diving into naming conventions, it's crucial to determine whether your module provides multiple distinct features or if it serves a single, focused purpose. This distinction directly influences how you should name your functions, types, and files.

- **Multiple Distinct Features:** If your module offers several unrelated features, each feature should be named distinctly within the module. For example, in a CAN abstraction layer, you might have functionality dedicated to CAN messages and other features like cyclic data timeouts. Functions and types related to these features should be named accordingly
    * Ex. `CAL_MSG_MCBDiagnosticTypeDef` or `CAL_CYCLIC_set_data()`.
    * Ex (This one is super relevant). `HAL_CAN_*` vs `HAL_ADC_*`.

- **Single Purpose Module:** If your module has a singular focus, then your naming should be straightforward and directly related to the module's functionality. For example, in an RTC module that handles time-related operations, you might name functions like `RTC_get_current_timestamp()`.


With this distinction aside lets discuss various naming conventions that pop up in modules

#### File Names
- **Naming Convention:**
  - Both `.c` and `.h` files should be named after the module, using `snake_case` and lowercase.
  - Examples:
    - If abstracting LCD screen functionality: `lcd.c` and `lcd.h`.
    - If creating a utility functions module: `utils.c` and `utils.h`.
  - Generally, both the `.c` and `.h` file should share the same name.

#### Function Names
- **Public Functions:**
  - For multiple distinct features, use `<MODULE_NAME_CAPITALIZED>_<FEATURE>_<function_name>`.
    - Ex: For a CAN abstraction layer (CAL) function transmitting with the `MCBDiagnostic` message, name the function `CAL_MSG_Tx_MCBDiagnostic(data)`.
  - For single-purpose modules, use `<MODULE_NAME_CAPITALIZED>_<function_name>`.
    - Ex: For an RTC module, name a function `RTC_get_current_timestamp()`.

- **Private Functions:**
  - Name private functions without the module name, using only `<function_name>`.
    - Example: If `RTC_get_current_timestamp()` calls private functions, name them `get_date_epoch_seconds()` and `get_time_epoch_seconds()`.
    - *Note: Do not add a `_` prefix to private funciton names*. 

#### Type Names
- **Naming Convention:**
  - Follow the pattern `<MODULE_NAME_CAPITALIZED>_<FEATURE>_TypeDef`.
  - Example: In a CAN abstraction layer, the `TELDiagnostic` message struct which holds its signals, is named `CAL_MSG_TELDiagnosticTypeDef`.
    - The reason we do a `typedef`, even though there are some debates against doing this, is because we are writing with `HAL` code which *already uses typedefs* for their defintions. Thus, to maintain one standard in our firmware, we will use `typedef`. 
    - Additionally, we add `TypeDef` at the end of the type definiton so that we maintain the standard `HAL` has already set out. For example, `HAL` will do typedefs like `CAN_TxHeaderTypeDef`.

### Naming Convention for Local and Global Variables
#### Global Variables
- **Naming Convention:**
  - Global variables should be prefixed with `g_` to immediately indicate it is global, the module name in uppercase, followed by an underscore, and then a descriptive name in `snake_case`.
  - Ex:
    - The global normalized throttle in MCB code: `g_MCB_throttle`.

#### Local Variables
- **Naming Convention:**
  - Local variables should be named using `snake_case` and be as descriptive as possible without becoming overly verbose.
  - Since local variables are scoped within a function or block, there's no need to prefix them with the module name.
  - Ex:
    - `gps_data`, `can_msg`, `curr_timestamp`.

#### Constants
- **Naming Convention:**
  - Constants should be named in uppercase with words separated by underscores.
  - Prefix the constant with the module name if it is global, or just the descriptive name if it is local.
  - Ex:
    - Global: `CAL_TELDIAGNOSTIC_ID`.
    - Local: `MASK_8_BIT`.


### Commenting

#### Motivation
Why should we comment our code? Well let me explain with an example:
```c
uint32_t start_of_second = 0;
```
When you read this code (previously in the `main.c` of the `tel/` firmware when initializing globals) it is not clear what `uint32_t start_of_second` is used for. You could guess it just initiliazes the start time of the board to 0 seconds but then the variable name `start_of_second` does not make sense because you would expect a name like `start_of_board`. 

This is where commenting comes in. 
```c
// Used to sync milliseconds upon receiving the 0x300 message (1s interval) to correct TEL board clock drift
uint32_t start_of_second = 0;
```
Now, it makes much more sense why this variable even exists; its goal is to reduce the TEL board's clock drift from impacting measurements taken over the whole board's runtime down to 1 second. 

The sections below will explain the following
* Function Headers
* General Commenting Rules


#### Function Headers
- **Naming Convention:**
  - Function headers should be written in the following format:
    ```c
    /**
     * @brief Brief description of the function
     * 
     * Detailed description of the function. 
     * Mention all modifications to any global variables or any other side effects.
     * Additionally mention all pre-conditions and post-conditions.
     * 
     * @param param1 Description of the first parameter
     * @param param2 Description of the second parameter
     * @return Description of the return value
     */
    ```
    - Example:
        ```c
        /**
         * @brief Transmits the given CAN message using the provided CAN handle and mailbox
         * 
         * It also handles the error cases where the mailbox is full or the message is not transmitted.
         * 
         * @param hcan: The CAN handle to use for transmission
         * @param msg the CAN message to transmit
         * @param can_mailbox The address of the mailbox to use for transmission
         * @return HAL_StatusTypeDef The status of the transmission
         */
        HAL_StatusTypeDef CAN_radio_and_bus_transmit(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *msg, uint32_t *can_mailbox);
        ```
        Note that there is no need to put the type of the parameters in the comments because the function signature already does that.

#### General Commenting Rules
**Do's:**
- **Clarify Intent:** Use comments to explain the purpose of non-obvious variables. The example in the `Motivation` section exemplifies this.
- **Provide Context:** Explain why certain decisions were made, *especially* if they might seem unusual or counterintuitive. For example:
  ```c
    CAN_radio_and_bus_transmit(&hcan, latitude_msg, &can_mailbox);
    osDelay(GPS_SINGLE_MSG_DELAY);      // Single delay between Txes so that Tx FIFO mailbox does not overfill and get overrun

    CAN_radio_and_bus_transmit(&hcan, longitude_msg, &can_mailbox);
    osDelay(GPS_SINGLE_MSG_DELAY);
    
    CAN_radio_and_bus_transmit(&hcan, altitude_hdop_msg, &can_mailbox);
    osDelay(GPS_SINGLE_MSG_DELAY);

    CAN_radio_and_bus_transmit(&hcan, side_and_count_msg, &can_mailbox);
    osDelay(GPS_NEXT_MSG_DELAY);
  ```
- **Use Comments Sparingly:** Comment only when necessary to add clarity or context, not to restate what the code already clearly expresses.
- **Prefer Comments on Same Line:** When possible, place comments on the same line as the code they describe if the comment is short and concise. Otherwise, place the comment on the line above the code. Longer comments usually occur in setup functions and shorter comments occur in processing or loops.

**Don'ts:**
- **Avoid Redundancy:** Don’t use comments that simply restate what the code does. For example:
  ```c
  i++;  // Increment i by 1  // Useless comment
  ```
  This is a useless comment that does explain the *why*. Note: even in this case the *why* is likely obvious so no comment is necessary,
- **Don't Comment the Obvious:** Avoid comments that explain self-explanatory code. For example:
  ```c
  CAN_handle_rx_msg(rx_CAN_msg);  // Handle the CAN message  // Useless comment
  ```
  The function name here does the same job as the comment. So delete the comment.
- **Avoid Outdated Comments:** Keep comments up to date with code changes. Outdated comments can be misleading. **We enforce this by taking initiative to update the outdated comments when we encounter them.**  


### Formatting

#### Variable Initialization
- **All at the top:**
  - Initialize all variables at the top of the function. Even if they are used later.
  - Example:
    ```c
    void CAN_diagnostic_msg_tx_radio_bus() {
        uint8_t data_send = INITIAL_FLAGS;                  // We set all variables here
        CAN_Radio_msg_t diagnostics_msg;
        diagnostics_msg.header = tel_diagnostics_header;
        union Utils_DoubleBytes_t current_timestamp;

        current_timestamp.double_value = get_current_timestamp();
        diagnostics_msg.timestamp = current_timestamp;

        if(g_tel_diagnostics.rtc_reset)                  
          SET_BIT(data_send, FLAG_HIGH << 0);
        if(g_tel_diagnostics.gps_fix)
          SET_BIT(data_send, FLAG_HIGH << 1);
        if(g_tel_diagnostics.imu_fail)
          SET_BIT(data_send, FLAG_HIGH << 2);
        if(g_tel_diagnostics.watchdog_reset)
          SET_BIT(data_send, FLAG_HIGH << 3);
        
        diagnostics_msg.data[FIRST_DATA_BYTE] = data_send;
    }
    ```
    
#### Indentation
- **Indentation Style:**
  - Use 4 spaces for indentation. STM32CubeIDE might default to 2 spaces.
    - Change this by following: Window -> Preferences -> C/C++ -> Code Style -> Formatter -> Edit -> Indentation -> Tab policy -> Spaces only -> Indentation size -> 4. 
  - Example:
    ```c
    void example_function() {
        if (condition) {
            // Do something
        } 
        else {
            // Do something else
        }
    }
    ```

#### Line Length
- **Maximum Line Length:**
  - Limit all lines to 120 characters.
  - If a line exceeds this limit, consider breaking it up across multiple lines, ensuring readability.
  - Example:
    ```c
    int result = very_long_function_name(argument1, argument2, argument3,
                                         argument4, argument5, argument6);
    ```

#### Whitespace
- **Spaces Around Operators:**
  - Use a single space around binary operators (`=`, `+`, `-`, etc.) for readability.
  - Do not use spaces around unary operators (e.g., `!`, `++`, `--`).
  - Examples:
    ```c
    int sum = a + b;
    int flag = !condition;
    sum++;
    ```

- **Spacing Inside Parentheses:**
  - Do not add spaces immediately inside parentheses, brackets, or braces.
    - Do this:
      ```c
      if (condition) {
          array[index] = value;
      }
      ```
    - Not this:
      ```c
      if ( condition ) {
          array[ index ] = value;
      }
      ```
- **Spacing After Commas:**
  - Use a single space after commas in function arguments and array initializers.
  - Example:
    ```c
    int array[] = {1, 2, 3, 4};
    ```
- **Spacing After Conditions**
  - Use a single space after the `if`, `else`, `for`, `while`, and `switch` keywords. Also a single space after the condition which comes before the `{`.
  - Example:
    ```c
    if (condition) {
        // Do something
    }
    ```

#### Blank Lines
- **Blank Lines Usage:**
  - Use blank lines to separate logical sections of code to enhance readability.
  - Example:
    ```c
    uint8_t data_send = INITIAL_FLAGS;
    CAN_Radio_msg_t diagnostics_msg;
    diagnostics_msg.header = tel_diagnostics_header;
    union Utils_DoubleBytes_t current_timestamp;

    current_timestamp.double_value = get_current_timestamp();
    diagnostics_msg.timestamp = current_timestamp;

    if(g_tel_diagnostics.rtc_reset)                   // Only set TEL's diagnostic flags bit by bit
      SET_BIT(data_send, FLAG_HIGH << 0);
    if(g_tel_diagnostics.gps_fix)
      SET_BIT(data_send, FLAG_HIGH << 1);
    if(g_tel_diagnostics.imu_fail)
      SET_BIT(data_send, FLAG_HIGH << 2);
    if(g_tel_diagnostics.watchdog_reset)
      SET_BIT(data_send, FLAG_HIGH << 3);
    
    diagnostics_msg.data[FIRST_DATA_BYTE] = data_send;
    ```
    Here we separate initialization of variables, setting the timestamp, setting the data bits, and setting the data byte with blank lines to make it easier to read.
