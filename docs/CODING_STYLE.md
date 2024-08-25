# Coding Style
Solar's coding style can be broken down into two parts. 
* [Macro styles](#macro-style)
    * This standard explains hows to organize your new feature and utilize the concept of software components.
* [Micro styles](#micro-style)
    * This standard explains when to comment, naming conventions, and any line specific standards.


## Macro Style
The sections below include
* [Overview](#overview)
* [Translation Units](#translation-units)
	* [Example of a Translation Unit](#example-of-a-translation-unit)
	* [The One Definition Rule (ODR) and its Application](#the-one-definition-rule-odr-and-its-application)
* [Abstracting Functionality](#abstracting-functionality)
	* [Example of Abstracting Functionality](#example-of-abstra	cting-functionality)
* [Creating a Software Component (Module)](#creating-a-software-component-module)
	* [Identify the Functionality](#1-identify-the-functionality)
		* **Single Responsibility Principle**
	* [Create the Header File (`.h`)](#2-create-the-header-file-h)
		* **Include Guard**
		* **Public Function Declarations**
		* **Symbolic Constants**
	* [Create the Source File (`.c`)](#3-create-the-source-file-c)
		* **Abstract Repetitive Operations**	
	* [Organize Your Modules](#4-organize-your-modules)
		* **Project-Specific Modules**
		* **General-Purpose Modules**


### Overview

The Macro Style section covers the high-level organizational structure of your codebase. It emphasizes the importance of organizing code into reusable software components, also known as modules, to promote portability and scalability. This section will guide you through creating, organizing, and naming these modules, as well as setting up the file structure for your projects. 

### Translation Units

A translation unit in C refers to a source file after it has been preprocessed (your preprocessor directives like `#include` or `#define` have been resolved) and is ready to be compiled into an object file. It includes the source file itself, along with all the header files and other dependencies that are included.

Each translation unit is compiled separately by the compiler, and the object files generated from these units are later linked together to form the final executable. Understanding translation units is important especially when dealing with multiple files and modules because it helps in maintaining the One Definition Rule (ODR) and avoiding potential linker errors.

#### Example of a Translation Unit

Consider the following files:

**main.c:**
```c
#include "module.h"

int main(void) {
    myFunction();
    return 0;
}
```

**module.h:**
```c
#ifndef MODULE_H
#define MODULE_H

void myFunction(void);

#endif // MODULE_H
```

**module.c:**
```c
#include "module.h"
#include <stdio.h>

void myFunction(void) {
    printf("Hello from myFunction!\n");
}
```

In this example:

- **`main.c`** is a source file that includes the header file `module.h` and calls the function `myFunction`.
- **`module.h`** is a header file that *declares* the function `myFunction`.
- **`module.c`** is a source file that *defines* the function `myFunction`.

When you compile the code, each source file (`main.c` and `module.c`) is *treated as a separate translation unit*. The preprocessor will replace `#include "module.h"` with the contents of `module.h`, and then the compiler will process each translation unit independently.

#### The One Definition Rule (ODR) and its Application

The One Definition Rule (ODR) ensures that a function or global variable is defined only once across all translation units in a program. While *declarations* can appear in multiple places (e.g., in different header files), the actual *definition* must be unique. This rule prevents conflicts and ambiguities during the linking stage, which could otherwise result in linker errors.

In the example above:

- The function `myFunction` is *declared* in `module.h` but *defined* in `module.c`. This adheres to the ODR because the function is **defined only once** across all translation units.
	* *If we were to define `myFunction` in both `main.c` and `module.c`, it would violate the ODR, potentially causing a linker error due to multiple definitions of the same function.*

Always adhere to the ODR. It ensures that our code is well-structured, maintainable, and free of common linkage issues.

### Abstracting Functionality

When writing code, aim to abstract repetitive operations into functions and encapsulate related functions within modules. This promotes code reuse, reducing memory usage, easier to read code, and makes the codebase easier to maintain as well. **Always put in the extra-effort to make your code clean because it goes a long way!**.

#### Example of Abstracting Functionality

Instead of writing the same code repeatedly:
```c
if(g_tel_diagnostics.rtc_reset)                  
	SET_BIT(data_send, FLAG_HIGH << RTC_RESET_BIT);
if(g_tel_diagnostics.gps_fix)
	SET_BIT(data_send, FLAG_HIGH << GPS_FIX_BIT);
if(g_tel_diagnostics.imu_fail)
	SET_BIT(data_send, FLAG_HIGH << IMU_FAIL_BIT);
if(g_tel_diagnostics.watchdog_reset)
	SET_BIT(data_send, FLAG_HIGH << WATCHDOG_RESET_BIT);
```
This can be abstracted into a function:
```c
void UTIL_set_bit_on_flag(uint8_t *byte, uint8_t bit_idx, bool condition) {
	if (condition) {
		SET_BIT(*byte, FLAG_HIGH << bit_idx);
	}
}

// Usage
UTIL_set_bit_on_flag(&data_send, RTC_RESET_BIT     , g_tel_diagnostics.rtc_reset);
UTIL_set_bit_on_flag(&data_send, GPS_FIX_BIT       , g_tel_diagnostics.gps_fix);
UTIL_set_bit_on_flag(&data_send, IMU_FAIL_BIT      , g_tel_diagnostics.imu_fail);
UTIL_set_bit_on_flag(&data_send, WATCHDOG_RESET_BIT, g_tel_diagnostics.watchdog_reset);
```
Now, you can call this function four times, making your code more concise and easier to maintain. Additionally, since this is a generic function, you can reuse it in other parts of your codebase by making it a public utility function. *Note: The function header is not shown in this, however, should be there*.

### Creating a Software Component (Module)

A software component, or module, is a self-contained unit of code that performs a specific function or set of related functions. The goal of a module is to abstract functionality into source (`.c`) and header (`.h`) files, making it easy to reuse across different projects. 

#### 1. **Identify the Functionality**
Determine the specific task or set of tasks your module will perform. For example, if you're creating a module to handle CAN communication, all CAN-related functions should be encapsulated within this module and not require external dependencies (this would kill portability).
- **Single Responsibility Principle**: Each module should have a single responsibility or focus. This makes the module easier to understand, test, and maintain.

#### 2. **Create the Header File (`.h`)**: 
Once you know what functionality you want to provide, make a header file. This file will serve as the interface for your module. It should declare all public functions, types, and constants that other modules or files **need** to access.
- **Include Guard**: Protect your header file from multiple inclusions by using an include guard. For modules we create do not use surrounding underscores. Instead, do:
	```c
	#ifndef CAN_MODULE_H
	#define CAN_MODULE_H

	// Function, type, and other Declarations

	#endif // CAN_MODULE_H
	```
- **Public Function Declarations**: Declare all public functions in the header file, ensuring they are accessible to other modules.
	- **Example**:
		```c
		void RADIO_tx_CAN_msg(CAN_Radio_msg_t *tx_CAN_msg);
		```

- **Public Function Declarations**: Declare all public functions in the header file, ensuring they are accessible to other modules. Remember that each function should be defined in exactly one source file to adhere to the ODR.

- **Symbolic Constants**: If a constant is meant to be used across different files, declare it in the header file with the module name prefixed to it. Example: `#define CAN_MAX_MSG_BYTES 8`.
	- If a constant is only used within the `.c` file, declare it in the `.c` file only instead of the header to avoid unnecessary exposure.

#### 3. **Create the Source File (`.c`)**: 
This file contains the actual implementation of the module's functionality. It should include the corresponding header file and define all the functions and variables declared in the header.
- **Abstract Repetitive Operations**: If you have repetitive operations in your code, consider abstracting them into separate functions as shown in the [Abstracting Functionality](#example-of-abstracting-functionality) section.

#### 4. **Organize Your Modules**: 
Place your modules in appropriate directories based on their purpose:
- **Project-Specific Modules**: These modules are specific to a particular project and should be placed within the project's directory.
	- Ex. If you are abstracting GPS module functionality for the TEL board then add the `gps.c` source file in `tel/Core/Src/` and the `gps.h` header file in `tel/Core/Inc/`.
- **General-Purpose Modules**: If a module is designed to be reused across multiple projects, place it in the `library/` folder. This way, it can be easily included in any project as needed.


## Micro Style
The sections below include
* [Naming Convention for Software Components (Modules)](#naming-convention-for-software-components-modules)
	* [Understanding Your Module](#understanding-your-module)
	* [File Names](#file-names)
	* [Function Names](#function-names)
	* [Type Names](#type-names)
* [Naming Convention for Local and Global Variables](#naming-convention-for-local-and-global-variables)
	* [Global Variables](#global-variables)
	* [Local Variables](#local-variables)
	* [Constants](#constants)
* [Commenting](#commenting)
	* [Function Headers](#function-headers)
	* [General Commenting Rules](#general-commenting-rules)
* [Formatting](#formatting)
	* [Variable Initialization](#variable-initialization)
	* [Indentation](#indentation)
	* [Line Length](#line-length)
	* [Whitespace](#whitespace)
	* [Blank Lines](#blank-lines)


### Naming Convention for Software Components (Modules)
As explained in the Macro Style section, we use software components (synonymous with "modules" from here on out) to abstract functionality and enforce portability. While we briefly mentioned how to set up the file structure and workflow of modules, this section explicitly covers the necessary naming conventions when writing these modules. 

*Note: If you are writing code inside a pre-existing component or module then these rules will still apply* 

#### Understanding Your Module

Before diving into naming conventions, it's crucial to determine whether your module provides multiple distinct features or if it serves a single, focused purpose. This distinction directly influences how you should name your functions, types, and files.

- **Multiple Distinct Features:** If your module offers several unrelated features, each feature should be named distinctly within the module. For example, in a CAN abstraction layer (`CAL` below), you might have functionality dedicated to CAN messages (`MSG` below) and other features like cyclic data timeouts. Functions and types related to these features should be named accordingly
    * Ex. `CAL_MSG_MCBDiagnosticTypeDef` or `CAL_CYCLIC_set_data()`.
    * Ex (This one is super relevant). `HAL_CAN_*`. HAL is the main module and CAN is a submodule. Also, `HAL_ADC_*` as another example.

- **Single Purpose Module:** If your module has a singular focus, then your naming should be straightforward and directly related to the module's functionality. For example, in an RTC module that handles time-related operations, you might name functions like `RTC_get_current_timestamp()`.


With this distinction aside lets discuss various naming conventions that pop up in modules

#### File Names
- **Naming Convention:**
  - Both `.c` and `.h` files should be named after the module, using `snake_case` and lowercase.
  - Examples:
    - If abstracting LCD screen functionality: `lcd.c` and `lcd.h`.
    - If creating a utility functions module: `utils.c` and `utils.h`.
    - Multiple words: `sd_logger.c` and `sd_logger.h`.
  - Generally, both the `.c` and `.h` file should share the same name.

#### Function Names
- **Public Functions:**
  - For multiple distinct features, use `<MODULE_NAME_CAPITALIZED>_<FEATURE>_<function_name>`.
    - Ex: For a CAN abstraction layer (CAL) function that transmits the `MCBDiagnostic` message, name the function `CAL_MSG_Tx_MCBDiagnostic`.
  - For single-purpose modules, use `<MODULE_NAME_CAPITALIZED>_<function_name>`.
    - Ex: For an RTC module, name a function `RTC_get_current_timestamp()`.

- **Private Functions:**
  - Name private functions without the module name, using only `<function_name>`.
    - Example: If `RTC_get_current_timestamp()` calls private functions, name them `get_date_epoch_seconds()` and `get_time_epoch_seconds()`.
    - *Note: Do **not** add a `_` prefix to private funciton names*. 

#### Type Names
- **Naming Convention:**
  - Follow the pattern `<MODULE_NAME_CAPITALIZED>_<FEATURE>_TypeDef`.
  - Example: In a CAN abstraction layer, the `TELDiagnostic` message struct which holds its signals, is named `CAL_MSG_TELDiagnosticTypeDef`.
    - The reason we do a `typedef`, even though there are some arguements *against* doing this, is because we are writing with `HAL` code which *already uses typedefs* for their defintions. Thus, to maintain one standard in our firmware, we will use `typedef`. 
    - Additionally, we add `TypeDef` at the end of the type definiton so that we maintain the standard `HAL` has already set out. For example, `HAL` will do typedefs like `CAN_TxHeaderTypeDef`.

### Naming Convention for Local and Global Variables
#### Global Variables
- **Naming Convention:**
  - Global variables should be prefixed with `g_` to immediately indicate it is global, the module name in uppercase, followed by an underscore, and then a descriptive name in `snake_case`.
  - Ex:
    - The global variable representive the normalized throttle value in MCB code: `g_MCB_throttle`.

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
When you read this code (which was in the `main.c` of the `tel/` firmware) it is not clear what `uint32_t start_of_second` is used for. You could guess it just initiliazes the start time of the board to 0 seconds but then the variable name `start_of_second` does not make sense because you would expect a name like `start_of_board`. 

This is where commenting comes in. 
```c
// Used to sync milliseconds upon receiving the 0x300 message (on 1s intervals) to minimize impact of TEL board clock drift
// Ex. Drift = 0.1% error over 1 hour = 3.6s error vs 0.001s error on 1s intervals.
uint32_t start_of_second = 0;
```
Now, it makes more sense why this variable even exists; its goal is to reduce the TEL board's clock drift from affecting measurements taken over the whole board's runtime (could be hours during competition) down to only impacting 1 second.

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
- **Clarify Intent:** Use comments to explain the purpose of non-obvious variables. The example in the [Motivation](#motivation) section exemplifies this.
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
- **Use Comments Sparingly:** Comment only when necessary to add clarity or context, not to restate what the code already clearly expresses. For example, in the example directly above, the comment will stop developers from removing a seemingly unnecessary delay when it is actually necessary.
- **Prefer Comments on Same Line:** When possible, place comments on the same line as the code they describe if the comment is short and concise. Otherwise, place the comment on the line above the code. Longer comments usually occur in setup functions to explain why certain variables were made and shorter comments occur in processing or loops.

**Don'ts:**
- **Avoid Redundancy:** Don’t use comments that simply restate what the code does. For example:
  ```c
  i++;  // Increment i by 1  // Useless comment
  ```
  This is a useless comment that does explain the *why*. Note: even in this case the *why* is likely obvious so no comment is even necessary in the first place,
- **Don't Comment the Obvious:** Avoid comments that explain self-explanatory code. For example:
  ```c
  CAN_handle_rx_msg(rx_CAN_msg);  // Handle the CAN message  // Useless comment
  ```
  The function name here does the same job as the comment. *Note: GitHub Copilot probably autogenerated that comment so if you use it get used to deleting some stuff it creates for you*. A better comment would be:
  ```c
  CAN_handle_rx_msg(rx_CAN_msg);  // Attempts to RTC sync, extracts into a Radio Rx struct, then sends msg over UART
  ```
- **Avoid Outdated Comments:** Keep comments up to date with code changes. Outdated comments can be misleading. **We enforce this by taking initiative to update the outdated comments when we encounter them.** For example, the CAN ID comment here is outdated because the RTC timestamp id is 0x300 not 0x200.
  ```c 
  /* Perform rtc syncing check if the message is 0x200 and if RTC is reset to 2000-01-01 */
  if (rx_CAN_msg->header.StdId == RTC_TIMESTAMP_MSG_ID && checkAndSetRTCReset())
  {
      sync_memorator_rtc(rx_CAN_msg);
  }
  ```


### Formatting

#### Variable Initialization
- **All at the top:**
  - Initialize all variables at the top of the function. Even if they are used later.
  - Example:
    ```c
    void CAN_diagnostic_msg_tx_radio_bus() {
        uint8_t data_send = INITIAL_FLAGS;                  // We set all variables here. Note we dont actually comment this.
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

## Conclusion
The coding style standards outlined in this document are designed to promote consistency, readability, and maintainability across the Solar firmware repos. By adhering to these standards, we ensure our code is clean, well-organized, and easy to understand for both yourself, new recruits, and any contributor. Remember that these standards are not set in stone and can be adapted as needed to suit the specific requirements of your project. However, **consistency is key**, so be sure to follow these guidelines throughout your codebase to maintain a high level of quality and professionalism. Happy coding! 
