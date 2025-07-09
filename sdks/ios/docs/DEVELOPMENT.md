# DeliteAI iOS SDK

To build the **DeliteAI iOS SDK** locally and run tests follow these steps below:

## Running the SDK Locally Through Example App

To effectively test and observe the functionalities of the DeliteAI iOS SDK locally, you can utilize the provided `NimbleNetExample` application. Follow these detailed steps to get it up and running:

1.  **Navigate to the Example Directory:**
    Open your terminal or command prompt and change your current directory to the `example` folder located within your cloned DeliteAI iOS SDK repository.

2.  **Install Dependencies via CocoaPods:**
    From within the `example` directory, execute the following command. This will fetch all necessary dependencies, including the DeliteAI iOS SDK itself from your local code, ensuring you're testing against your current development version.

    ```bash
    pod install
    ```

3.  **Open the Xcode Workspace:**
    Once the `pod install` command completes successfully, open the generated Xcode workspace file. It's crucial to open the `.xcworkspace` file, not the `.xcodeproj` file, to ensure all pods are correctly linked.

    ```bash
    open NimbleNetExample.xcworkspace
    ```

4.  **Select the Target:**
    In Xcode, ensure that `NimbleNetExample` is selected as the active target for compilation and running. You can find this dropdown menu next to the play/stop buttons in the Xcode toolbar.

5.  **Build and Run the Application:**
    With the target selected, click the **Run** (play) button in Xcode. Choose your desired simulator or a connected iOS device. As the app runs, closely observe the **Debug Area**. This output will help you verify the core functionalities of the SDK.

## Running Unit Tests

The `NimbleNetExample` project includes a dedicated test target, `NimbleNetExample_Tests`, with several unit tests to ensure the robustness and correctness of the DeliteAI iOS SDK's functionalities. Developers can run these tests to verify specific components and behaviors.

The following test files are available:

1. RunMethodInstrumentation.swift
2. ProtoTest.swift
3. Keychaintest.swift

To run these test cases:

1.  **Open the Xcode Workspace:**
    If not already open, open `NimbleNetExample.xcworkspace` in Xcode as described in the installation steps.

2.  **Navigate to Test Files:**
    In Xcode's Project Navigator (left sidebar), locate the `Test` directory. Expand it to find the source files, including `RunMethodInstrumentation.swift`, `ProtoTest.swift`, and `Keychaintest.swift`.

3.  **Execute Specific Tests:**
    You have a few options to run the tests:
    * **Run a Single Test Class:** Open any of the test files (e.g., `RunMethodInstrumentation.swift`). You will see a small diamond-shaped "play" button next to the `class` declaration line. Clicking this button will run all test methods within that specific class.
    * **Run a Single Test Function:** Within a test file, each individual `func` (test method) also has a diamond-shaped "play" button next to its declaration. Clicking this will execute only that particular test function.

4.  **Observe Test Results:**
    After running tests, Xcode will display the results in the **Debug Area** and the **Test Navigator** (accessible via `Cmd + 6`). Successful tests will be marked with a green checkmark, while failures will show a red 'X' along with details about the assertion that failed.

