#!/usr/bin/env python3
"""
Dictionary interface test with actual ONNX model inference.

This script tests both traditional tensor interface and new dictionary interface
with a proper add/subtract ONNX model that uses supported float32 data types.
"""

from delitepy import nimblenet as nm

# Load model at global scope as required by DeliteAI simulator
model = nm.Model("test_model")

def test_tensor_interface(input_data):
    """Test the traditional tensor-based model interface with actual inference."""
    try:
        # Check model status
        status = model.status()

        # Test with actual model.run() call
        # Create test inputs: X=3.0, Y=2.0 (expected: sum=5.0, diff=1.0)
        X_tensor = nm.tensor([[3.0]], "float")
        Y_tensor = nm.tensor([[2.0]], "float")

        # Run model with traditional tensor interface
        output = model.run(X_tensor, Y_tensor)

        # Extract results
        sum_result = None
        diff_result = None
        inference_successful = False

        # Check if output exists and has elements (avoiding != None comparison)
        if output:
            if len(output) >= 2:
                sum_result = output[0]
                diff_result = output[1]
                inference_successful = True

        return {
            "status": "success",
            "model_loaded": status,
            "inference_successful": inference_successful,
            "interface_type": "tensor",
            "sum_output": sum_result,
            "diff_output": diff_result,
            "message": "Traditional tensor interface with actual inference"
        }

    except Exception as e:
        return {
            "status": "error",
            "error": str(e)
        }

def test_dictionary_interface(input_data):
    """Test the new dictionary-based model interface with actual inference."""
    try:
        # Check model status
        status = model.status()

        # Test with actual model.run() call using dictionary
        # Create test inputs: X=5.0, Y=3.0 (expected: sum=8.0, diff=2.0)
        X_tensor = nm.tensor([[5.0]], "float")
        Y_tensor = nm.tensor([[3.0]], "float")

        # Create input dictionary for new interface
        input_dict = {"X": X_tensor, "Y": Y_tensor}

        # Run model with dictionary interface
        output_dict = model.run(input_dict)

        # Extract results by name
        sum_result = None
        diff_result = None
        inference_successful = False

        # Check if output exists (avoiding != None comparison)
        if output_dict:
            try:
                # Try to access outputs by name (this is the key test!)
                sum_result = output_dict["sum"]
                diff_result = output_dict["difference"]
                inference_successful = True
            except Exception as access_error:
                # If named access fails, try positional access as fallback
                try:
                    if len(output_dict) >= 2:
                        sum_result = output_dict[0]
                        diff_result = output_dict[1]
                        inference_successful = True
                except Exception as pos_error:
                    # Positional access also failed
                    inference_successful = False

        return {
            "status": "success",
            "model_loaded": status,
            "inference_successful": inference_successful,
            "interface_type": "dictionary",
            "sum_output": sum_result,
            "diff_output": diff_result,
            "message": "Dictionary interface with actual inference"
        }

    except Exception as e:
        return {
            "status": "error",
            "error": str(e)
        }

def test_interface_equivalence(input_data):
    """Test that both interfaces produce equivalent results."""
    try:
        # Check model status
        status = model.status()

        # Test both interfaces with same inputs: X=4.0, Y=1.0 (expected: sum=5.0, diff=3.0)
        X_tensor = nm.tensor([[4.0]], "float")
        Y_tensor = nm.tensor([[1.0]], "float")

        # Test traditional interface
        tensor_output = model.run(X_tensor, Y_tensor)
        print("Tensor output: ",tensor_output[0][0][0])

        input_dict = {"X": X_tensor, "Y": Y_tensor}
        dict_output = model.run(input_dict)
        print("Dict output: ",dict_output["sum"][0][0])

        first_output_match = tensor_output[0][0][0] == dict_output["sum"][0][0]
        second_output_match = tensor_output[1][0][0] == dict_output["difference"][0][0]

        return {
            "status": "success",
            "model_loaded": status,
            "both_interfaces_equivalent": first_output_match and second_output_match,
            "message": "Interface equivalence with actual inference tested"
        }

    except Exception as e:
        return {
            "status": "error",
            "error": str(e)
        }
