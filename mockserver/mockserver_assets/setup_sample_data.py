# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import requests
import base64
import os
import time
import sys

# Sample Data:
# CLIENTS: testclient and prodclient pairs.
# MODELS: Addition by 2 model and by 3 model with version 1.0.0 and 2.0.0 respectively under add_model name and multiplication_two_model
# SCRIPTS: version 1 with just addition function, version 2 with multiplication function, version 3 with both addition and multiplication in same function, version 4 with no model execution
# Following are the deployments for each compatibility created:

# MODEL_CHANGE deployment id1 : Add_model with version 1.0.0 and script version 1.0.0
# MODEL_CHANGE deployment id2 : Multiplication_two_model with script version 2.0.0

# MODEL_UPDATE deployment id1: Add_model with version 1.0.0 and script version 1.0.0
# MODEL_UPDATE deployment id2: Add_model with version 2.0.0 and script version 1.0.0

# MODEL_ADDITION deployment id1: Add_model with version 1.0.0 with script version 1.0.0
# MODEL_ADDITION deployment id2: Add_model with version 1.0.0, Multiplication_two_model with script version 3.0.0

# NO_MODEL deployment id1: No models with script version 4.0.0

DMS_HOST = os.getenv("REMOTE_DMS_URL")
TEST_CLIENT = "testclient"
PROD_CLIENT = "prodclient"
TEST_USER = "test@delite-ai.dev"
COMPATIBILITY_TAG_PYTHON_MODULES = "PYTHON_MODULES"
COMPATIBILITY_TAG_MODEL_CHANGE = "MODEL_CHANGE"
COMPATIBILITY_TAG_MODEL_UPDATE = "MODEL_UPDATE"
COMPATIBILITY_TAG_MODEL_ADDITION = "MODEL_ADDITION"
COMPATIBILITY_TAG_NO_MODEL = "NO_MODEL"
COMPATIBILITY_TAG_ADD_EVENT = "ADD_EVENT"
COMPATIBILITY_TAG_SCRIPT_LOAD_FAILURE = "SCRIPT_LOAD_FAILURE"
COMPATIBILITY_TAG_ADD_EVENT_PROTO = "ADD_EVENT_PROTO"
COMPATIBILITY_TAG_LLM = "LLM"
COMPATIBILITY_TAG_LIST_COM_LLMS = "LIST_COM_LLMS"

ORG_ID = 1
ADD_TWO_MODEL_FILE_NAME = "add_two_model.onnx"
ADD_THREE_MODEL_FILE_NAME = "add_three_model.onnx"
ADD_MODEL_NAME = "add_model"
MULTIPLY_TWO_MODEL_FILE_NAME = "multiply_two_model.onnx"
MULTIPLY_TWO_MODEL_NAME = "multiply_two_model"
LLM_MODEL_FILE_PATH = "llama-3.zip"
LLM_MODEL_NAME = "llama-3"
SCRIPT_1_FILE_NAME = "add_script.py"
SCRIPT_2_FILE_NAME = "multiply_script.py"
SCRIPT_3_FILE_NAME = "add_and_multiply_script.py"
SCRIPT_4_FILE_NAME = "no_model_script.py"
ADD_EVENT_SCRIPT_FILE_NAME = "add_event_script.py"
SCRIPT_6_FILE_NAME = "no_model_bad_script.py"
ADD_EVENT_SCRIPT_PROTO_FILE_NAME = "add_event_proto_script.py"
LLM_SCRIPT_FILE_NAME = "chatbot.py"
LIST_COMPATIBLE_LLM_FILE_NAME = "list_compatible_llms.py"
DEFAULT_SCRIPT = "DEFAULT_SCRIPT"
SCRIPT_7_FILE_NAME = "python_modules.zip"

# Wait for DMS to be healthy before calling APIs
def wait_for_healthz():
    healthz_url = f"{DMS_HOST}/dms/healthz"
    max_retries = 5
    retry_delay = 5
    for _ in range(max_retries):
        try:
            response = requests.get(healthz_url)
            if response.status_code == 200:
                print("DMS service is up. Proceeding with adding mock data")
                return
            else:
                print(f"Healthz check failed")
        except Exception as e:
            print(f"Error connecting to healthz endpoint: {e}")
        print(f"Retrying in {retry_delay} seconds...")
        time.sleep(retry_delay)
    
    print(f"Healthz endpoint check failed after {max_retries} retries. Exiting script.")
    sys.exit(1)

# Exit script if response has non success status code
def handle_response(response):
    if not response.ok:
        print(f"Error response from API: {response.status_code} - {response.text}")
        sys.exit(1)

def create_org():
    print("Creating test organization")
    org_data = {"name": "TestOrg"}
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v1/private/organization", json=org_data))

def create_client_pair():
    print("Creating sample client pair")
    client_pair_data = {
        "organizationId": ORG_ID,
        "prodClient": {
            "id": PROD_CLIENT,
            "name": "Prod Client",
            "reviewCount": 1,
            "clientSecret": "dummmy",
            "cliClientSecret": "dummy"
        },
        "testClient": {
            "id": TEST_CLIENT,
            "name": "Test Client",
            "reviewCount": 0,
            "clientSecret": "dummy",
            "cliClientSecret": "dummy"
        }
    }
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v1/private/clientpair", json=client_pair_data))

def create_test_user():
    print(f"Creating org admin with email {TEST_USER}")
    user_data = {"email": TEST_USER, "permission": "admin"}
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v1/private/organizations/user", headers={"organizationId": str(ORG_ID)}, json=user_data))

def set_cloud_config():
    print(f"Setting cloud config for {TEST_CLIENT}")
    config_data = {
        "clientId": TEST_CLIENT,
        "sdkVersion": "v4",
        "config": {
            "nimbleLogger": {
                "sender": {
                    "interval": 1,
                    "key": "apikey",
                    "maxConcurrentLogFailures": 3,
                    "maxFilesToSend": 5,
                    "sendFirstLog": True,
                    "sendLogsProbability": 1,
                    "url": "http://localhost:8080/logger"
                },
                "writer": {
                    "logTypesToWrite": {
                        "acu": True,
                        "debug": True,
                        "info": True
                    },
                    "maxLogFileSizeKB": 1
                }
            },
            "externalLogger": {
                "sender": {
                    "url": "http://localhost:8080/externalLogger",
                    "interval": 1,
                    "sendFirstLog": True,
                    "key": "apikey"
                },
                "writer": {
                    "collectEvents": False
                }
            },
            "requestToHostMap": {
                "cloudConfig": "service",
                "model": "service",
                "register_event": "service",
                "script": "service",
                "llm": "service",
            }
        }
    }
    handle_response(requests.put(f"{DMS_HOST}/dms/api/v1/private/client/config", headers={"clientId": TEST_CLIENT}, json=config_data))

common_headers = {
    "clientid": TEST_CLIENT,
    "EmailId": TEST_USER,
    "organizationId": str(ORG_ID)
}

# Upload model
def upload_model(modelFileName, modelName):
    print(f"Uploading {modelName} model")
    with open(f"{modelFileName}", "rb") as f:
        model_base64 = base64.b64encode(f.read()).decode("utf-8")
    model_data = {
        "format": "onnx",
        "name": modelName,
        "isPrivate": False,
        "description": "Test Model",
        "metadata": {"epConfigVersion": 1, "epConfigs": {"android": [{"providerName": "XNNPACK", "runtime": "onnx"}]}},
        "data": model_base64,
        "type": "model"
    }
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v3/admin/asset", headers=common_headers, json=model_data))

# Update model
def update_model(modelFileName, modelName):
    print(f"Updating {modelName} model")
    with open(f"{modelFileName}", "rb") as f:
        model_base64 = base64.b64encode(f.read()).decode("utf-8")
    model_data = {
        "format": "onnx",
        "name": modelName,
        "description": "Test Model",
        "metadata": {},
        "data": model_base64,
        "updateType": 1,
        "isPrivate": False,
        "type": "model",
    }
    handle_response(requests.put(f"{DMS_HOST}/dms/api/v3/admin/asset", headers=common_headers, json=model_data))

def upload_script(scriptFileName, scriptName):
    # Upload script
    print(f"Uploading script with fileName: {scriptFileName}")
    ext = os.path.splitext(scriptFileName)[1][1:]
    if ext == "py":
        ext = "python"
    with open(scriptFileName, "rb") as f:
        taskCode = base64.b64encode(f.read()).decode("utf-8")
    script_data = {
        "name": scriptName,
        "data": taskCode,
        "description": "Test Script",
        "format": ext,
        "metadata": {},
        "type": "script"
    }
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v3/admin/asset", headers=common_headers, json=script_data))

def update_script(scriptFileName, scriptName):
    print(f"Updating script with fileName: {scriptFileName}")
    ext = os.path.splitext(scriptFileName)[1][1:]
    if ext == "py":
        ext = "python"
    # Updating script
    with open(scriptFileName, "rb") as f:
        taskCode = base64.b64encode(f.read()).decode("utf-8")
    script_data = {
        "name": scriptName,
        "data": taskCode,
        "description": "Test Script",
        "updateType": 1,
        "format": ext,
        "isPrivate": False,
        "metadata": {},
        "type": "script"
    }
    handle_response(requests.put(f"{DMS_HOST}/dms/api/v3/admin/asset", headers=common_headers, json=script_data))

# Upload LLM
def upload_llm(llmZipFileName, llmName):
    with open(f"{llmZipFileName}", "rb") as f:
        model_base64 = base64.b64encode(f.read()).decode("utf-8")
    print("Conversion to base64 done")
    model_data = {
        "format": "zip",
        "name": llmName,
        "isPrivate": False,
        "description": "Test Model",
        "metadata": {},
        "data": model_base64,
        "type": "llm"
    }
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v3/admin/asset", headers=common_headers, json=model_data))

# Create compatibility tag
def create_compatibility_tag(tagName):
    print("Creating compatibility tag")
    tag_data = {"name": tagName, "description": "default tag"}
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v3/admin/compatibilityTag", headers=common_headers, json=tag_data))

# Create deployment
def create_deployment(tag, modules, scriptModule):
    print("Creating deployment")
    deployment_data = {
        "compatibilityTag": tag,
        "modules": modules,
        "task": scriptModule,
        "name": "test deployment",
        "description": "test deployment"
    }
    handle_response(requests.post(f"{DMS_HOST}/dms/api/v3/admin/deployment", headers=common_headers, json=deployment_data))


if __name__ == "__main__":
    wait_for_healthz()

    # Create Organization
    create_org()

    # Create client pairs
    create_client_pair()

    # Create test user
    create_test_user()

    # Set cloud config
    set_cloud_config()

    # Upload models
    upload_model(ADD_TWO_MODEL_FILE_NAME, ADD_MODEL_NAME)
    upload_model(MULTIPLY_TWO_MODEL_FILE_NAME, MULTIPLY_TWO_MODEL_NAME)
    update_model(ADD_THREE_MODEL_FILE_NAME, ADD_MODEL_NAME)

    # Upload llm
    upload_llm(LLM_MODEL_FILE_PATH, LLM_MODEL_NAME)

    # Upload script versions
    upload_script(SCRIPT_1_FILE_NAME, DEFAULT_SCRIPT)
    update_script(SCRIPT_2_FILE_NAME, DEFAULT_SCRIPT)
    update_script(SCRIPT_3_FILE_NAME, DEFAULT_SCRIPT)
    update_script(SCRIPT_4_FILE_NAME, DEFAULT_SCRIPT)
    update_script(ADD_EVENT_SCRIPT_FILE_NAME, DEFAULT_SCRIPT)
    update_script(SCRIPT_6_FILE_NAME, DEFAULT_SCRIPT)
    update_script(ADD_EVENT_SCRIPT_PROTO_FILE_NAME, DEFAULT_SCRIPT)
    update_script(LLM_SCRIPT_FILE_NAME, DEFAULT_SCRIPT)
    update_script(SCRIPT_7_FILE_NAME, DEFAULT_SCRIPT)
    update_script(LIST_COMPATIBLE_LLM_FILE_NAME, DEFAULT_SCRIPT)
    
    # Create compatibility tag
    create_compatibility_tag(COMPATIBILITY_TAG_PYTHON_MODULES)
    create_compatibility_tag(COMPATIBILITY_TAG_MODEL_CHANGE)
    create_compatibility_tag(COMPATIBILITY_TAG_MODEL_ADDITION)
    create_compatibility_tag(COMPATIBILITY_TAG_MODEL_UPDATE)
    create_compatibility_tag(COMPATIBILITY_TAG_NO_MODEL)
    create_compatibility_tag(COMPATIBILITY_TAG_ADD_EVENT)
    create_compatibility_tag(COMPATIBILITY_TAG_SCRIPT_LOAD_FAILURE)
    create_compatibility_tag(COMPATIBILITY_TAG_ADD_EVENT_PROTO)
    create_compatibility_tag(COMPATIBILITY_TAG_LLM)
    create_compatibility_tag(COMPATIBILITY_TAG_LIST_COM_LLMS)

    # Create deployments
    create_deployment(COMPATIBILITY_TAG_MODEL_CHANGE, [{"name": ADD_MODEL_NAME, "version": "1.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "1.0.0", "type": "script"})
    create_deployment(COMPATIBILITY_TAG_MODEL_CHANGE, [{"name": MULTIPLY_TWO_MODEL_NAME, "version": "1.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "2.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_MODEL_UPDATE, [{"name": ADD_MODEL_NAME, "version": "1.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "1.0.0", "type": "script"})
    create_deployment(COMPATIBILITY_TAG_MODEL_UPDATE, [{"name": ADD_MODEL_NAME, "version": "2.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "1.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_MODEL_ADDITION, [{"name": ADD_MODEL_NAME, "version": "1.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "1.0.0", "type": "script"})
    create_deployment(COMPATIBILITY_TAG_MODEL_ADDITION, [{"name": ADD_MODEL_NAME, "version": "1.0.0", "type": "model"}, {"name": MULTIPLY_TWO_MODEL_NAME, "version": "1.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "3.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_NO_MODEL, [], {"name": DEFAULT_SCRIPT, "version": "4.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_ADD_EVENT, [], {"name": DEFAULT_SCRIPT, "version": "5.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_SCRIPT_LOAD_FAILURE, [], {"name": DEFAULT_SCRIPT, "version": "4.0.0", "type": "script"})
    create_deployment(COMPATIBILITY_TAG_SCRIPT_LOAD_FAILURE, [], {"name": DEFAULT_SCRIPT, "version": "6.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_ADD_EVENT_PROTO, [], {"name": DEFAULT_SCRIPT, "version": "7.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_LLM, [{"name": LLM_MODEL_NAME, "version": "1.0.0", "type": "llm"}], {"name": DEFAULT_SCRIPT, "version": "8.0.0", "type": "script"})
    create_deployment(COMPATIBILITY_TAG_PYTHON_MODULES, [{"name": ADD_MODEL_NAME, "version": "1.0.0", "type": "model"}], {"name": DEFAULT_SCRIPT, "version": "9.0.0", "type": "script"})

    create_deployment(COMPATIBILITY_TAG_LIST_COM_LLMS, [{"name": LLM_MODEL_NAME, "version": "1.0.0", "type": "llm"}], {"name": DEFAULT_SCRIPT, "version": "10.0.0", "type": "script"})

    print("Mock data added successfully!")
