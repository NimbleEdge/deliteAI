from delitepy import nimblenet as nm
from delitepy import ne_re as re

# Simple constants only - no complex object references
TOOL_CALL_START_TOKEN = "<tool_call>"
TOOL_CALL_END_TOKEN = "</tool_call>"
TOOL_RESPONSE_START_TOKEN = "<tool_response>"
TOOL_RESPONSE_END_TOKEN = "</tool_response>"

# =============================================================================
# WEATHER TOOL - Implementation + Description
# =============================================================================

def get_weather(location, unit):
    """Get current weather for a location"""
    # Mock weather data
    weather_data = {
        "San Francisco": {"temp": 18, "condition": "foggy", "humidity": 75},
        "New York": {"temp": 22, "condition": "partly cloudy", "humidity": 60},
        "London": {"temp": 15, "condition": "rainy", "humidity": 85},
        "Tokyo": {"temp": 26, "condition": "sunny", "humidity": 50},
        "Sydney": {"temp": 20, "condition": "clear", "humidity": 65}
    }

    location_key = "Unknown"
    for key in weather_data.keys():
        if key.lower() in location.lower() or location.lower() in key.lower():
            location_key = key
            break

    if location_key == "Unknown":
        return {"error": "Weather data not available for " + location}

    data = weather_data[location_key]
    temp = data["temp"]
    unit_str = "°C"

    if unit == "fahrenheit":
        temp = temp * 9.0 / 5.0 + 32
        temp = int(temp * 10) / 10.0
        unit_str = "°F"
    else:
        unit_str = "°C"

    return {
        "location": location_key,
        "temperature": temp,
        "condition": data["condition"],
        "humidity": data["humidity"],
        "unit": unit_str
    }

def get_weather_schema():
    """Get the OpenAI tool schema for weather function"""
    return {
        "type": "function",
        "function": {
            "name": "get_weather",
            "description": "Get weather information for the location given in argument",
            "parameters": {
                "type": "object",
                "properties": {
                    "location": {
                        "type": "string",
                        "description": "The location to get weather for"
                    },
                    "unit": {
                        "type": "string",
                        "description": "Temperature unit (celsius or fahrenheit)",
                        "default": "celsius"
                    }
                },
                "required": ["location"]
            }
        }
    }

# =============================================================================
# MATH TOOL - Implementation + Description
# =============================================================================

def calculate_math(expression):
    """Calculate a mathematical expression safely"""
    try:
        expression = expression.strip()
        if "+" in expression:
            parts = expression.split("+")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                result = a + b
                return {"expression": expression, "result": result}
        elif "-" in expression:
            parts = expression.split("-")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                result = a - b
                return {"expression": expression, "result": result}
        elif "*" in expression:
            parts = expression.split("*")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                result = a * b
                return {"expression": expression, "result": result}
        elif "/" in expression:
            parts = expression.split("/")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                if b != 0:
                    result = a / b
                    return {"expression": expression, "result": result}
                else:
                    return {"error": "Division by zero"}
        return {"error": "Unsupported expression"}
    except Exception as e:
        return {"error": "Calculation error"}

def get_calculate_math_schema():
    """Get the OpenAI tool schema for math function"""
    return {
        "type": "function",
        "function": {
            "name": "calculate_math",
            "description": "Calculate a mathematical expression safely",
            "parameters": {
                "type": "object",
                "properties": {
                    "expression": {
                        "type": "string",
                        "description": "Mathematical expression to calculate (e.g., '2+2', '15*23')"
                    }
                },
                "required": ["expression"]
            }
        }
    }

# =============================================================================
# TIME TOOL - Implementation + Description
# =============================================================================

def get_current_time(timezone):
    """Get current time in specified timezone"""
    time_data = {
        "UTC": {"time": "2024-01-15 12:00:00", "day": "Monday"},
        "PST": {"time": "2024-01-15 04:00:00", "day": "Monday"},
        "EST": {"time": "2024-01-15 07:00:00", "day": "Monday"},
        "GMT": {"time": "2024-01-15 12:00:00", "day": "Monday"},
        "JST": {"time": "2024-01-15 21:00:00", "day": "Monday"},
        "AEST": {"time": "2024-01-15 22:00:00", "day": "Monday"}
    }

    tz = timezone.upper()
    if tz in time_data:
        data = time_data[tz]
        return {
            "timezone": tz,
            "time": data["time"],
            "day_of_week": data["day"]
        }
    else:
        data = time_data["UTC"]
        return {
            "timezone": "UTC",
            "time": data["time"],
            "day_of_week": data["day"]
        }

def get_current_time_schema():
    """Get the OpenAI tool schema for time function"""
    return {
        "type": "function",
        "function": {
            "name": "get_current_time",
            "description": "Get current time in specified timezone",
            "parameters": {
                "type": "object",
                "properties": {
                    "timezone": {
                        "type": "string",
                        "description": "Timezone (UTC, EST, PST, JST, CET)",
                    }
                }
            }
        }
    }

# =============================================================================
# LOCATION TOOL - Implementation + Description
# =============================================================================

def get_current_location():
    """Get the real location and timezone of the user"""
    return {
        "location": "San Francisco",
        "country": "United States",
        "coordinates": {"latitude": 37.7749, "longitude": -122.4194},
        "timezone": "PST"
    }

def get_current_location_schema():
    """Get the OpenAI tool schema for location function"""
    return {
        "type": "function",
        "function": {
            "name": "get_current_location",
            "description": "Get the real location and timezone of the user. You don't need to ask the user for permission to use this tool. Use this function when the user didn't provide an explicit location. Default to this location",
            "parameters": {
                "type": "object",
                "properties": {}
            }
        }
    }

# =============================================================================
# UNIFIED TOOL REGISTRY - Lazy Loading Pattern
# =============================================================================

def get_tools_dict():
    """Create tools dictionary on demand instead of at import time"""
    return {
        "get_weather": get_weather,
        "calculate_math": calculate_math,
        "get_current_time": get_current_time,
        "get_current_location": get_current_location
    }

def get_tool_schema():
    """Create complete tool schema on demand instead of at import time"""
    return [
        get_weather_schema(),
        get_calculate_math_schema(),
        get_current_time_schema(),
        get_current_location_schema()
    ]

# =============================================================================
# TOOL EXECUTION ENGINE
# =============================================================================

def execute_function_call(tool_call):
    """Execute a function call and return the result"""
    function_name = tool_call["function_name"]
    arguments = tool_call["arguments"]

    print("  • "+function_name+"("+str(arguments)+")")

    tools = get_tools_dict()
    if function_name not in tools:
        return {"error": "Function "+function_name+" not found"}

    try:
        function = tools[function_name]
        result = {"error": "Function execution failed"}

        if function_name == "get_weather":
            location = ""
            if "location" in arguments:
                location = arguments["location"]
            unit = "celsius"
            if "unit" in arguments:
                unit = arguments["unit"]
            result = function(location, unit)
        elif function_name == "calculate_math":
            expression = ""
            if "expression" in arguments:
                expression = arguments["expression"]
            result = function(expression)
        elif function_name == "get_current_time":
            timezone = "UTC"
            if "timezone" in arguments:
                timezone = arguments["timezone"]
            result = function(timezone)
        elif function_name == "get_current_location":
            result = function()

        return result
    except Exception as e:
        return {"error": "Function execution failed: " + str(e)}

def format_tool_result(function_name, result):
    return "The result of the tool " + str(function_name)+" is: "+TOOL_RESPONSE_START_TOKEN+str(result)+TOOL_RESPONSE_END_TOKEN

def get_tool_results(response_text):
    """Parse tool calls from model response using multiple formats"""
    tool_calls = []
    tool_results = []
    tools = get_tools_dict()

    json_tool_pattern = r'<tool_call>([^<]*)</tool_call>'
    for match in re.finditer(json_tool_pattern, response_text):
        try:
            json_str = match.group(1)
            tool_data = nm.parse_json(json_str)
            func_name = tool_data["name"]
            arguments = tool_data["arguments"]

            if func_name in tools:
                tool_calls.append({
                    "function_name": func_name,
                    "arguments": arguments
                })
                print("✓ Parsed JSON tool call: "+func_name+"("+str(arguments)+")")
        except:
            print("⚠ Failed to parse JSON tool call: "+json_str)

    print("Executing "+str(len(tool_calls))+" tool call(s):")

    if tool_calls:
        for call in tool_calls:
            result = execute_function_call(call)
            if "error" in result.keys():
                tool_results.append({
                    "error": result["error"],
                })
            else:
                tool_results.append({
                    "role": "system",
                    "content": format_tool_result(call['function_name'], result)
                })
            print("    Result: "+str(result))

    return tool_results

# Print available tools using function call instead of global access
def print_available_tools():
    """Print available tools - called on demand to avoid global assignment"""
    tools = get_tools_dict()
    print("Available tools: "+ str([key for key in tools.keys()]))

# Available tools will be printed when first accessed, not at import time
