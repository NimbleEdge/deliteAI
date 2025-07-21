# DeliteAI Agents Android Example

Example Android app demonstrating the Notifications Summarizer Agent integration.

## Agents integrated in the Example App.

| Agent | README |
|-------|--------|
| Notifications Summarizer | [Android README](../../notifications_summarizer/android/README.md) |

## Setup

1. **Run the following commands**

   **Change to repo root**
   ```bash
   cd "$(git rev-parse --show-toplevel)"
   ```

2. **Download Llama ONNX assets**
   ```bash
   python3 coreruntime/tests/utils/download_from_s3.py \
     --default_bucket deliteai \
     --prefix build-dependencies/llama-3.2-1B/onnx \
     --output agents/examples/android/app/src/main/assets/llama-3 \
     --archive_output True
   ```

3. **Generate Python AST and move it into your Android assets**
   ```bash
   python3 coreruntime/scripts/gen_python_ast.py \
     agents/notifications_summarizer/delitepyAssets/main.py \
     && mv "${_%.py}.ast" agents/examples/android/app/src/main/assets/
   ```

4. **Open in Android Studio**
   ```bash
   cd deliteAI/agents/examples/android
   ```
   Open this directory in Android Studio.

5. **Sync and Build**  
   Let Gradle sync, then build the project.

## Running the App

1. Open agents/examples/android in Android Studio
2. Connect device or start emulator
2. Click Run button

## First Launch

1. **Grant Notification Access**: App will prompt for notification access permission
2. **Optional**: Disable battery optimization for reliable background processing
3. **SDK Auto-initializes**: App automatically sets up DeliteAI SDK and agent

## Features

The app demonstrates:
- **Schedule**: Schedules summary job for 10 seconds later
- **Summarize Current**: Gets summary of active notifications
- **By ID**: Retrieves specific summary by ID
- **Today/Last 7 Days**: Historical summary retrieval

## Expected Output

```
Overview: You received 5 notifications today from messaging apps.

Urgent Notifications:
📱 WhatsApp: Meeting reminder message

App-Wise Summary:
com.whatsapp
Meeting reminder message.
Group chat has 2 new messages.
``` 
