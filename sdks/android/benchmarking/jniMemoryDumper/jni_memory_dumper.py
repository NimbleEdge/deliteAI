# NOTE:
# 1. Go to the build tab in android studio and click on Build APK(s)
# 2. Install the required python dependency from requirements.txt
# 3. Start appium server using appium --allow-insecure chromedriver_autodownload

import time
import subprocess
import re
import pandas as pd
import os
from appium import webdriver
from appium.options.android import UiAutomator2Options
from appium.webdriver.common.appiumby import AppiumBy

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
APK_RELATIVE_PATH = "../../app/build/outputs/apk/external/debug/app-external-debug.apk"
APK_FILE = os.path.abspath(os.path.join(SCRIPT_DIR, APK_RELATIVE_PATH))
print(f"Resolved APK Path: {APK_FILE}")

PACKAGE_NAME = "ai.nimbleedge.android.sampleapp.debug"
MEMORY_COMMAND = f"adb shell dumpsys meminfo {PACKAGE_NAME}"
CHECK_PROCESS_COMMAND = f"adb shell pidof {PACKAGE_NAME}"

# Install the APK
print("Installing app.apk...")
install_result = subprocess.run(["adb", "install", "-r", APK_FILE], capture_output=True, text=True)
if "Success" not in install_result.stdout:
    print("Failed to install APK:", install_result.stderr)
    exit(1)

print("App installed successfully.")

# Launch the app
print("Launching the app...")
subprocess.run(["adb", "shell", "monkey", "-p", PACKAGE_NAME, "-c", "android.intent.category.LAUNCHER", "1"])

# Wait for the app to start
time.sleep(3)

# Set up Appium driver with UiAutomator2Options
options = UiAutomator2Options()
options.platform_name = "Android"
options.automation_name = "UiAutomator2"
options.app_package = PACKAGE_NAME
options.app_activity = ".MainActivity"  
options.no_reset = True

# Connect to Appium server
APPIUM_SERVER = "http://127.0.0.1:4723"  # Set the correct appium port
driver = webdriver.Remote(APPIUM_SERVER, options=options)

# Function to check if "PREDICT" text is visible
def wait_for_predict_text(timeout=120):
    print("Waiting for 'PREDICT' to appear on the screen...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            predict_element = driver.find_element(AppiumBy.XPATH, "//*[contains(@text, 'PREDICT')]")
            if predict_element:
                print("'PREDICT' detected! Starting memory monitoring...")
                return True
        except:
            time.sleep(0.5)
    print("Timed out waiting for 'PREDICT'. Exiting.")
    return False

# Wait for "PREDICT" text before proceeding
if not wait_for_predict_text():
    driver.quit()
    exit(1)

memory_data = {
    "Java Heap": [],
    "Native Heap": [],
    "Code": [],
    "Stack": []
}

memory_pattern = {
    "Java Heap": re.compile(r"Java Heap:\s+\d+\s+(\d+)"),
    "Native Heap": re.compile(r"Native Heap:\s+\d+\s+(\d+)"),
    "Code": re.compile(r"Code:\s+\d+\s+(\d+)"),
    "Stack": re.compile(r"Stack:\s+\d+\s+(\d+)"),
}

# Start monitoring memory
print("Monitoring memory usage... Press Ctrl+C to stop manually.")
start_time = time.time()

while True:
    # Check if the app is still running
    process_result = subprocess.run(CHECK_PROCESS_COMMAND.split(), capture_output=True, text=True)
    if not process_result.stdout.strip():
        print("\nApp has exited. Stopping monitoring.")
        break

    try:
        memory_info = subprocess.run(MEMORY_COMMAND.split(), capture_output=True, text=True).stdout

        # Extract values using regex
        for key, pattern in memory_pattern.items():
            match = pattern.search(memory_info)
            memory_data[key].append(int(match.group(1)) if match else 0)

    except Exception as e:
        print("Error fetching memory info:", e)
        break

    time.sleep(0.1)  # 100ms interval

df = pd.DataFrame(memory_data)

def get_last_non_zero(series):
    non_zero_values = series[series != 0]
    return non_zero_values.iloc[-1] if not non_zero_values.empty else 0

summary = {
    "Parameter": df.columns,
    "Start Value": df.iloc[0].tolist(),
    "End Value": [get_last_non_zero(df[col]) for col in df.columns],
    "Peak Value": df.max().tolist()
}

# Print Summary Table
summary_df = pd.DataFrame(summary)
print("\n=== Memory Usage Summary ===")
print(summary_df.to_string(index=False))
