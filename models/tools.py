
import datetime
import inspect
from typing import get_origin, get_args, Union

# Initialize empty tool schema and tools mapping
tool_schema = []
tools = {}

def tool(func_or_description=None, **param_descriptions):    
    """
    Decorator to automatically generate tool schema from function signature and add to registry.
    
    Can be used both with and without parentheses:
        @tool
        def my_function(): ...
        
        @tool()
        def my_function(): ...
        
        @tool("Custom description")
        def my_function(): ...
    
    Args:
        func_or_description: Either a function (when used as @tool) or description string (when used as @tool())
        **param_descriptions: Optional parameter descriptions as keyword arguments.
    """
    def create_tool_definition(func, description=None):
        """Helper function to create tool definition from function"""
        # Get function name
        func_name = func.__name__
        
        # Get description from parameter or docstring
        func_description = description or (func.__doc__ or f"Execute {func_name}").strip()
        
        # Get function signature
        sig = inspect.signature(func)
        
        # Build parameters schema
        properties = {}
        required = []
        
        for param_name, param in sig.parameters.items():
            # Skip *args and **kwargs
            if param.kind in (param.VAR_POSITIONAL, param.VAR_KEYWORD):
                continue
                
            # Determine parameter type
            param_type = "string"  # default
            
            if param.annotation != param.empty:
                annotation = param.annotation
                
                # Handle Union types (like Optional[str])
                if get_origin(annotation) is Union:
                    args = get_args(annotation)
                    # Remove NoneType for Optional types
                    non_none_args = [arg for arg in args if arg is not type(None)]
                    if non_none_args:
                        annotation = non_none_args[0]
                
                # Map Python types to JSON schema types
                if annotation in (str, type(str)):
                    param_type = "string"
                elif annotation in (int, type(int)):
                    param_type = "integer"
                elif annotation in (float, type(float)):
                    param_type = "number"
                elif annotation in (bool, type(bool)):
                    param_type = "boolean"
                elif annotation in (list, type(list)):
                    param_type = "array"
                elif annotation in (dict, type(dict)):
                    param_type = "object"

            # Build parameter schema
            param_schema = {
                "type": param_type,
                "description": param_descriptions.get(param_name, f"The {param_name} parameter")
            }
                            
            # Check if parameter has default value
            if param.default != param.empty:
                param_schema["default"] = param.default
            else:
                required.append(param_name)
            properties[param_name] = param_schema
        
        # Build complete tool definition
        tool_definition = {
            "type": "function",
            "function": {
                "name": func_name,
                "description": func_description,
                "parameters": {
                    "type": "object",
                    "properties": properties,
                    "required": required
                }
            }
        }
        
        # Add to registry
        tool_schema.append(tool_definition)
        tools[func_name] = func
        return func
    
    # Case 1: Used as @tool (without parentheses)
    # The function is passed as the first argument
    if callable(func_or_description) and hasattr(func_or_description, '__name__'):
        return create_tool_definition(func_or_description)
    
    # Case 2: Used as @tool() or @tool("description") (with parentheses)  
    # Return a decorator function
    else:
        description = func_or_description if isinstance(func_or_description, str) else None
        
        def decorator(func):
            return create_tool_definition(func, description)
        
        return decorator

# Define example tools/functions
@tool(
    description="Get weather information for a specific location",
    location="The location to get weather for",
    unit="Temperature unit (celsius or fahrenheit)"
)
def get_weather(location: str, unit: str = "celsius") -> dict:
    """Get current weather for a location"""
    
    weather_data = {
        "New York": {"temp": 22, "condition": "sunny", "humidity": 65},
        "London": {"temp": 15, "condition": "cloudy", "humidity": 78},
        "Tokyo": {"temp": 28, "condition": "rainy", "humidity": 85},
        "Paris": {"temp": 18, "condition": "partly cloudy", "humidity": 70}
    }
    
    location_key = next((key for key in weather_data.keys() if key.lower() in location.lower()), "Unknown")
    
    if location_key == "Unknown":
        return {"error": f"Weather data not available for {location}"}
    
    data = weather_data[location_key].copy()
    if unit == "fahrenheit":
        data["temp"] = round(data["temp"] * 9/5 + 32, 1)
        data["unit"] = "°F"
    else:
        data["unit"] = "°C"
    
    return {
        "location": location_key,
        "temperature": data["temp"],
        "condition": data["condition"],
        "humidity": data["humidity"],
        "unit": data["unit"]
    }

@tool(
    expression="Mathematical expression to calculate (e.g., '2+2', '15*23')"
)
def calculate_math(expression: str) -> dict:
    """Calculate a mathematical expression safely"""
    try:
        allowed_chars = set('0123456789+-*/.() ')
        if not all(c in allowed_chars for c in expression):
            return {"error": "Expression contains invalid characters"}
        
        result = eval(expression)
        return {"expression": expression, "result": result}
    except Exception as e:
        return {"error": f"Calculation error: {str(e)}"}

@tool(
    timezone="Timezone (UTC, EST, PST, JST, CET)"
)
def get_current_time(timezone: str = "UTC") -> dict:
    """Get current time in specified timezone"""
    current_time = datetime.datetime.now()
    timezone_offsets = {"UTC": 0, "EST": -5, "PST": -8, "JST": 9, "CET": 1}
    
    offset = timezone_offsets.get(timezone.upper(), 0)
    adjusted_time = current_time + datetime.timedelta(hours=offset)
    
    return {
        "timezone": timezone.upper(),
        "time": adjusted_time.strftime("%Y-%m-%d %H:%M:%S"),
        "day_of_week": adjusted_time.strftime("%A")
    }

@tool
def get_current_location() -> dict:
    """
    Get the real location and timezone of the user. You don't need to ask the user for permission to use this tool. 
    Use this function when the user didn't provide an explicit location. Default to this location
    """
    return {
        "location": "Tokyo",
        "country": "Japan",
        "coordinates": {"latitude": 35.6762, "longitude": 139.6503},
        "timezone": "JST"
    }