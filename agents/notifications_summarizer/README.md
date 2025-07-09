# Notifications Summarizer Agent

Automatically processes and summarizes device notifications using on-device language models. Transforms notification overload into actionable insights while maintaining privacy through local processing.

## How It Works

### Three-Stage Process
1. **Individual Summarization**: Each notification becomes a concise summary
2. **App Grouping**: Notifications grouped by application
3. **Holistic Overview**: Combined summary with daily overview, urgent items, and app-wise breakdown

### Features
- On-device processing (no data leaves device)
- Background scheduling
- Historical summary retrieval
- Real-time processing

### Example Output
Here is an example of the summary text produced by the agent:

```text
Here's a summary of your day's notifications.

**Urgent Notifications**
- ⚠️ com.example.bank: Your credit card has been charged $500.

**App-Wise Summary**
com.example.chat
- You have 2 new messages from Alice.
- A friend request from Bob was received.

com.example.fooddelivery
- Your burger order has been delivered.
```

## Platform Support

- [Android Implementation](android/README.md)
