# Initialize empty tool schema and tools mapping
tool_schema = []
tools_dict = {}

# Define tool functions
def get_weather(location, unit):
    """Get current weather for a location"""
    # Mock weather data - in real app would call weather API
    weather_data = {
        "San Francisco": {"temp": 18, "condition": "foggy", "humidity": 75},
        "New York": {"temp": 22, "condition": "partly cloudy", "humidity": 60},
        "London": {"temp": 15, "condition": "rainy", "humidity": 85},
        "Tokyo": {"temp": 26, "condition": "sunny", "humidity": 50},
        "Sydney": {"temp": 20, "condition": "clear", "humidity": 65}
    }
    
    # Simple location matching
    location_key = "Unknown"
    for key in weather_data.keys():
        if key.lower() in location.lower() or location.lower() in key.lower():
            location_key = key
            break
    
    if location_key == "Unknown":
        return {"error": "Weather data not available for " + location}
    
    data = weather_data[location_key]
    temp = data["temp"]
    unit_str = "°C"  # Initialize with default value
    
    if unit == "fahrenheit":
        temp = temp * 9.0 / 5.0 + 32
        temp = int(temp * 10) / 10.0  # Manual rounding to 1 decimal place
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

def calculate_math(expression):
    """Calculate a mathematical expression safely"""
    try:
        # Clean the expression
        expression = expression.strip()
        
        # Handle multiplication
        if "*" in expression:
            parts = expression.split("*")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                result = a * b
                return {"expression": expression, "result": result}
        
        # Handle addition
        if "+" in expression:
            parts = expression.split("+")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                result = a + b
                return {"expression": expression, "result": result}
        
        # Handle subtraction
        if "-" in expression:
            parts = expression.split("-")
            if len(parts) == 2:
                a = float(parts[0].strip())
                b = float(parts[1].strip())
                result = a - b
                return {"expression": expression, "result": result}
        
        # Handle division
        if "/" in expression:
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

def get_current_time(timezone):
    """Get current time in specified timezone"""
    # Mock time data - in real app would use proper timezone handling
    # Using fixed time values for simulation since we don't have datetime
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
        # Default to UTC if timezone not found
        data = time_data["UTC"]
        return {
            "timezone": "UTC",
            "time": data["time"],
            "day_of_week": data["day"]
        }

def get_current_location():
    """Get the real location and timezone of the user"""
    return {
        "location": "San Francisco",
        "country": "United States",
        "coordinates": {"latitude": 37.7749, "longitude": -122.4194},
        "timezone": "PST"
    }

# Create tools dictionary
tools_dict = {
    "get_weather": get_weather,
    "calculate_math": calculate_math,
    "get_current_time": get_current_time,
    "get_current_location": get_current_location
}

# Define tool schema
tool_schema = [
    {
        "type": "function",
        "function": {
            "name": "get_weather",
            "description": "Get weather information for a specific location",
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
    },
    {
        "type": "function",
        "function": {
            "name": "calculate_math",
            "description": "Calculate a mathematical expression safely",
            "parameters": {
                "type": "object",
                "properties": {
                    "expression": {
                        "type": "string",
                        "description": "Mathematical expression to calculate (supports +, -, *, /)"
                    }
                },
                "required": ["expression"]
            }
        }
    },
    {
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
                        "default": "UTC"
                    }
                }
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_current_location",
            "description": "Get the real location and timezone of the user",
            "parameters": {
                "type": "object",
                "properties": {}
            }
        }
    }
]