
# **CAN Communications Layer**

- [**CAN Communications Layer**](#can-communications-layer)
  - [**Overview**](#overview)
- [**Quick Start**](#quick-start)
  - [**Prerequisites**](#prerequisites)
  - [**Initialization**](#initialization)
  - [**Usage**](#usage)

## **Overview**

The CAN Communications Layer is a library that abstractes and simplifies the process of sending and receiving CAN messages by encapsulating CAN handling in a user-friendly API. This library leverages the HAL_CAN library for low-level CAN operations and FreeRTOS for managing asynchronous tasks, message queues, and semaphores.

# **Quick Start**

## **Prerequisites**

**STM32F1 series MCU**  

- This library was developed and tested with the STM32F1 series of MCUs. Other similar STM MCUs may work if they use the same underlying HAL.

**CAN Interrupts**  

- In the .ioc file, navigate to **Pinout & Configuration** -> **Connectivity** -> **CAN**.
- Check the **Activated** box.
- Check the **USB high priority or CAN TX interrupts** and **USB low priority or CAN RX0 interrupts** boxes.

**CMSIS_V2 FreeRTOS**  

- In the .ioc file, navigate to **Pinout & Configuration** -> **Middleware and Software Packs** -> **FREERTOS**.
- Select **CMSIS_V2** on the **Interface** drop down box.
- CAN_comms uses around 1024 bytes of heap space. You may need to increase the total heap size for FreeRTOS.

**Importing library**  

- See the **Libraries** section on the [wiki](https://wiki.ubcsolar.com/tutorials/stm32cubeide) for how to import a library to a project.

**Code Generator**  

- In the .ioc file, navigate to **Project Manager** -> **Code Generator**.
- Check the **Generate peripheral initialization as a pair of '.c/.h' files per peripheral** box.

## **Initialization**

**Include library**  

- Include the library using `#include "CAN_comms.h"`

**Create config struct**  

- Create a `CAN_comms_config_t` struct.

  ```c
  CAN_comms_config_t CAN_comms_config = {
      .hcan = &hcan,
      .CAN_Filter = CAN_filter,
      .CAN_comms_Rx_callback = CAN_comms_Rx_callback
    };
  ```

- `hcan` should be a pointer to the `CAN_HandleTypeDef` struct that's normally defined in `can.h`
- `CAN_Filter` should be a `CAN_FilterTypeDef` struct.
- `CAN_comms_Rx_callback` should be a pointer to a function of the form `void (*CAN_comms_Rx_callback)(CAN_comms_Rx_msg_t*);` This function will be called whenever a CAN message is received and ready to be parsed. This callback function can be defined anywhere, but it's recomended to define it in `can.c`.

**Initialize CAN Comms**  

- Call `CAN_comms_init()` and pass a pointer to the `CAN_comms_config_t` struct as the argument.
- Since `CAN_comms_init()` creates FreeRTOS tasks, it needs to be called after `osKernelInitialize()` and before `osKernelStart()`. The best spot to add it is in a user section in the `MX_FREERTOS_Init()` in `freertos.c`.

## **Usage**

**Receiving**

- CAN Rx messages are stored in memory as a `CAN_comms_Rx_msg_t` struct. Which contains a `CAN_RxHeaderTypeDef` and a `uint8_t` array parameters.
- The `CAN_comms_Rx_callback` function pointer is called from the `CAN_comms_Rx_task` whenever a CAN message is received. This is where the user impliments functionality for receiving CAN messages.

  ```c
  void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
  {
    // User implementation for parsing CAN_comms_Rx_msg
  }
  ```
  
**Transmitting**

- CAN Tx messages are stored in memory as a `CAN_comms_Tx_msg_t` struct. Which contains a `CAN_TxHeaderTypeDef` and a `uint8_t` array parameters.
- Call `CAN_comms_Add_Tx_message()` and pass in a pointer to the `CAN_comms_Tx_msg_t` struct to transmit a CAN message.

  ```c
  CAN_comms_Tx_msg_t CAN_comms_Tx_msg;

  CAN_comms_Tx_msg.header = //... Set header
  CAN_comms_Tx_msg.data =   //... Set data

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  ```

- `CAN_comms_Add_Tx_message()` will copy the `CAN_comms_Tx_msg` into a queue, so `CAN_comms_Tx_msg` can be discard once used.
