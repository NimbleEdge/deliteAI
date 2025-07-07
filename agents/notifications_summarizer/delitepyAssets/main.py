from delitepy import ne_re as re
from delitepy import nimblenet as nm

hardware_info = nm.get_hardware_info()
num_cores = int(hardware_info["numCores"])
nm.set_xnnpack_num_threads(int(num_cores / 2 + 1))

llm = nm.llm("llama-3")

# System / user prompts
SYSTEM_PROMPT_BEGIN = "<|start_header_id|>system<|end_header_id|>"
SYSTEM_PROMPT_TEXT = (
    "You are a notification summarizer. "
    "For every input notification, output exactly one concise sentence summary as plain text. "
    "Do not add any extra text, formatting, or JSON."
)
PROMPT_END = "<|eot_id|>\n"
SYSTEM_PROMPT = SYSTEM_PROMPT_BEGIN + SYSTEM_PROMPT_TEXT + PROMPT_END

USER_PROMPT_BEGIN = "<|start_header_id|>user<|end_header_id|>"
ASSISTANT_RESPONSE_BEGIN = "<|start_header_id|>assistant<|end_header_id|>"

MAX_CHARS_PER_NOTIFICATION = 2000  # kept in case you want to pre-truncate

@concurrent
def get_output_from_llm(message):
    output = ""
    final_prompt = SYSTEM_PROMPT + USER_PROMPT_BEGIN + message + PROMPT_END + ASSISTANT_RESPONSE_BEGIN
    print("final_prompt:", final_prompt)
    text_stream = llm.prompt(final_prompt)

    while text_stream is not None:
        chunk = text_stream.next()
        if chunk is None:
            break
        output = output + chunk
        if text_stream.finished():
            break

    return output.strip()

@concurrent
def summarize_single_notification(notification):
    """
    Step 1: Produce a single-line summary for a single notification.
    """
    raw_text = "Title: " + notification['title'] + "\nBody: " + notification['body']
    if len(raw_text) > MAX_CHARS_PER_NOTIFICATION:
        raw_text = raw_text[:MAX_CHARS_PER_NOTIFICATION] + "…"

    message = "Example:\n" + \
        "Input:\n" + \
        "Title: \"Food delivery is here\"\n" + \
        "Body: \"Your order from The Burger Place has arrived!\"\n" + \
        "Output:\n" + \
        "Your burger order has been delivered.\n\n" + \
        "Now summarize this:\n" + \
        "Title: \"" + notification['title'] + "\"\n" + \
        "Body: \"" + notification['body'] + "\"\n" + \
        "Respond with only a single-line summary:"
    return get_output_from_llm(message)

@concurrent
def summarize_app_group(package_name, single_summaries):
    """
    Step 2: Given a list of one-line summaries, prepend the app name as a heading.
    Returns plain text with app name and summaries.
    """
    joined = "\n".join(single_summaries)

    message = "Example:\n" + \
        "Input:\n" + \
        "App: com.example.chat\n" + \
        "Summaries:\n" + \
        "- You have 2 new messages.\n" + \
        "- Friend request received.\n" + \
        "Output:\n" + \
        "com.example.chat\n" + \
        "You have 2 new messages.\n" + \
        "Friend request received.\n\n" + \
        "Now do the same for:\n" + \
        "App: " + package_name + "\n" + \
        "Summaries:\n" + \
        joined + "\n\n" + \
        "Output exactly the format shown above:"
    return get_output_from_llm(message)

@concurrent
def generate_holistic_summary_overview(app_group_summaries):
    """
    Step 3: Build the final 3-section overview from all app-group blocks.
    Returns plain text with the overview.
    """
    joined_app_blocks = "\n\n".join(app_group_summaries)

    message = "Produce exactly three sections.\n" + \
        "1) Overview: concise sentence with entire day.\n" + \
        "2) Urgent Notifications: list only clearly urgent items with emoji, app name, and summary.\n" + \
        "3) App-Wise Summary: paste these blocks exactly:\n\n" + \
        "App Group Summaries:\n" + \
        joined_app_blocks + "\n\n" + \
        "Respond with plain text—no code fences, no extra headings, nothing else."
    return get_output_from_llm(message)

@concurrent
def summarize_notification(inp):
    try:
        notification_str = inp["input"]
        input_notifications = nm.parse_json(notification_str)
        print("input_notifications:", input_notifications)

        id = 0
        for i in input_notifications:
            i["id"] = str(id)
            id = id + 1

        print("phase 0:", input_notifications)

        # ===== Phase 1 =====
        single_summaries = {}
        for notif in input_notifications:
            res = summarize_single_notification(notif)
            single_summaries[notif["id"]] = res
            print("single_summary", res)

        print("phase 1:", single_summaries)

        # ===== Phase 2 =====
        notificationsByPackageName = {}

        for notification in input_notifications:
            pkg = notification["packageName"]

            if notification["id"] not in single_summaries:
                continue

            if pkg not in notificationsByPackageName:
                notificationsByPackageName[pkg] = []

            notificationsByPackageName[pkg].append(single_summaries[notification["id"]])

        print("phase 2:", notificationsByPackageName)

        # ===== Phase 3 =====
        app_group_summaries = []
        for package_name in notificationsByPackageName.keys():
            summaries_list = notificationsByPackageName[package_name]
            result = summarize_app_group(package_name, summaries_list)
            app_group_summaries.append(result)
            print("app_group_summary", result)

        print("phase 3:", app_group_summaries)

        # ===== Phase 4 =====
        res = generate_holistic_summary_overview(app_group_summaries)
        print("phase 4:", res)

        return {"output": str(res)}
    except Exception as e:
        print("Error processing notifications:", e)
        return {"output": "Error: " + str(e)}
