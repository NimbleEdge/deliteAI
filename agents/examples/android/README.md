<!-- DL_DOCS::WEBSITE_BLOCK_START
```{eval-rst}
:orphan:
```
DL_DOCS::WEBSITE_BLOCK_END -->

# DeliteAI Agents Android Example

Example Android app demonstrating the Notifications Summarizer Agent integration.

## Agents integrated in the Example App

| Agent | README |
|-------|--------|
| Notifications Summarizer | [Android README](../../notifications_summarizer/android/README.md) |

## Setup

1. **Add Credentials in Code**
   Open `app/src/main/java/dev/deliteai/examples/App.kt` and replace the placeholder values in the
   `NimbleNetConfig` block (`YOUR_CLIENT_ID`, `YOUR_HOST`, `YOUR_CLIENT_SECRET`) with your own
   credentials.

2. **Open in Android Studio**

   ```bash
   cd deliteAI/agents/examples/android
   ```

   Open this directory in Android Studio.

3. **Sync and Build**
   Let Gradle sync, then build the project.

## Running the App

1. Open agents/examples/android in Android Studio
2. Connect device or start emulator
3. Click Run button

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

```text
Overview: You received 5 notifications today from messaging apps.

Urgent Notifications:
📱 WhatsApp: Meeting reminder message

App-Wise Summary:
com.whatsapp
Meeting reminder message.
Group chat has 2 new messages.
```
