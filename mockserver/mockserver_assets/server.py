# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from flask import Flask, request, jsonify, make_response
import urllib3
from threading import Lock
import requests
import yaml
import argparse
import os
from time import sleep
from urllib.parse import urlparse, urlunparse
import json

app = Flask(__name__)

# Configuration loaded from a YAML file or environment variables
CONFIG = {
    "remote_dms_url": os.getenv("REMOTE_DMS_URL"),
    "remote_mds_url": os.getenv("REMOTE_MDS_URL"),
    "remote_log_url": os.getenv("LOGGER_URL"),
    "remote_external_logger_url": os.getenv("EXTERNAL_LOGGER_URL"),
    "ssl_private_key": os.getenv("SSL_PRIVATE_KEY"),  # Path to privkey.pem
    "ssl_full_chain": os.getenv("SSL_FULL_CHAIN"),    # Path to fullchain.pem
    "external_logger_events_file": "/tmp/ingestor/events.log",
    "external_logger_scriptlogs_file": "/tmp/ingestor/scriptlogs.log",
    "external_logger_unauthenticated_file": "/tmp/ingestor/unauthenticated.log"
}

# Thread-safe storage for mock expectations
mock_responses = {}
mock_lock = Lock()

# History of API call
historical_api_calls = []

# Endpoint to set mock responses
@app.route('/mocker/expectation', methods=['POST'])
def set_mock_response():
    data = request.json
    if "path" not in data:
        return jsonify({"error": "Missing required fields 'path' in request body."}), 400

    path = data["path"].split("?")[0]
    with mock_lock:
        mock_responses[path] = {
            "status_code": data.get("status_code", None),
            "headers": data.get("headers", {}),
            "body": data.get("body", ""),
            "repeat_count": data.get("repeat_count", None),
            "response_delay": data.get("response_delay", 0) / 1000.0  # Convert milliseconds to seconds
        }

    return jsonify({"message": "Mock response set successfully."}), 201

# Endpoint to delete mock responses
@app.route('/mocker/expectation', methods=['DELETE'])
def delete_mock_response():
    data = request.json
    if "path" not in data:
        return jsonify({"error": "Missing required field 'path' in request body."}), 400

    path = data["path"].split("?")[0]
    with mock_lock:
        if path in mock_responses:
            del mock_responses[path]
            return jsonify({"message": "Mock response deleted successfully."}), 200
        else:
            return jsonify({"error": "Mock response not found."}), 404

# Endpoint to fetch all mock responses
@app.route('/mocker/expectations', methods=['GET'])
def list_mock_responses():
    with mock_lock:
        return jsonify(mock_responses), 200

def clear_file(filePath):
    if os.path.exists(filePath):
        with open(filePath, "w") as file:
            pass

# Endpoint to reset all mock responses
@app.route('/mocker/reset', methods=['POST'])
def reset_mock_responses():
    with mock_lock:
        mock_responses.clear()
        historical_api_calls.clear() # Clear the history on mock server reset
        clear_file(CONFIG["external_logger_events_file"]) # Clear events file, only if it is present
        clear_file(CONFIG["external_logger_scriptlogs_file"]) # Clear scriptlogs file, only if present
        clear_file(CONFIG["external_logger_unauthenticated_file"]) # Clear unauthenticated external events/script logs file, only if present
    return jsonify({"message": "All mock responses have been reset."}), 200

def get_json_lines_from_file(filePath):
    json_lines = []
    if not os.path.exists(filePath):
        return json_lines
    with open(filePath) as file:
        for line in file:
            try:
                json_line = json.loads(line.strip())
                json_lines.append(json_line)
            except json.JSONDecodeError as e:
                raise(f"Invalid json object present in {filePath} with error: {e}")
    return json_lines

# Endpoint to fetch history of API calls made to mock server
@app.route('/mocker/history', methods=['GET'])
def list_history_api_calls():
    with mock_lock:
        return jsonify({"api_calls": historical_api_calls,
                        "external_logger_events": get_json_lines_from_file(CONFIG["external_logger_events_file"]),
                        "external_logger_scriptlogs": get_json_lines_from_file(CONFIG["external_logger_scriptlogs_file"]),
                        "unauthenticated_external_logs": get_json_lines_from_file(CONFIG["external_logger_unauthenticated_file"])}), 200

# Handle all other requests
@app.route('/', defaults={'path': ''}, methods=['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'])
@app.route('/<path:path>', methods=['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'])
def proxy_request(path):
    full_path = f"/{path}".split("?")[0]

    # Check for a matching mock response
    with mock_lock:
        mock = mock_responses.get(full_path)
        if mock:
            sleep(mock["response_delay"])  # Simulate response delay
            if mock["status_code"]:
                print(f"{full_path}: expectation matched, using mock response")
                response = make_response(mock["body"], mock["status_code"])
                for header, value in mock["headers"].items():
                    response.headers[header] = value
                historical_api_calls.append({"path": path, "status_code": mock["status_code"], "is_mocked": True})
                # Before sending response decrease repeat_count
                if mock["repeat_count"]:
                    mock["repeat_count"] -= 1
                    if mock["repeat_count"] == 0:
                        del mock_responses[full_path]
                return response

    print(f"{full_path}: expectation did not matched, forwarding request to proxy")

    proxy_path = full_path
    # Determine the target remote server based on path prefix
    if full_path.startswith("/mds"):
        remote_url = CONFIG["remote_mds_url"]
    elif full_path.startswith("/dms"):
        remote_url = CONFIG["remote_dms_url"]
    elif full_path.startswith("/logger"):
        remote_url = CONFIG["remote_log_url"]
        proxy_path = "/v2"
    elif full_path.startswith("/externalLogger"):
        remote_url = CONFIG["remote_external_logger_url"]
        proxy_path = "/v1/ingest"
    else:
        return jsonify({"error": "Path not found."}), 404

    # Forward request to the appropriate remote server
    parsed_url = urlparse(remote_url)
    target_url = urlunparse((parsed_url.scheme, parsed_url.netloc, proxy_path, '', '', ''))
    proxy_headers = {key: value for key, value in request.headers if key not in ['Host', 'Transfer-Encoding']}

    # requests library by default adds a 'Accept-Encoding: gzip' header to the request if not already present
    # This was causing the response returned to original client to be gzip compressed even when it was not expecting
    # Adding the SKIP_HEADER, prevents that default addition of Accept-Encoding header
    if 'Accept-Encoding' not in proxy_headers:
        proxy_headers['Accept-Encoding']= urllib3.util.SKIP_HEADER
    try:
        resp = requests.request(
            method=request.method,
            url=target_url,
            headers=proxy_headers,
            data=request.get_data(),
            params=request.args,
            cookies=request.cookies,
            allow_redirects=False,
            stream=True
        )
        response = make_response(resp.raw.read(), resp.status_code)
        for header, value in resp.headers.items():
            # Skip transfer-encoding header as it will be added automatically if response is chunked
            if header != "Transfer-Encoding":
                response.headers[header] = value
        with mock_lock:
            # Add forwarded request to history of API calls
            historical_api_calls.append({"path": path, "status_code": resp.status_code, "is_mocked": False})
        return response
    except requests.RequestException as e:
        return jsonify({"error": str(e)}), 502

if __name__ == "__main__":
    # Parse command-line arguments for config file path
    parser = argparse.ArgumentParser(description="Proxy and Mock Server")
    parser.add_argument("--config", type=str, default="./config.yaml", help="Path to the YAML configuration file")
    args = parser.parse_args()

    # Load configuration from the specified YAML file if it exists
    if os.path.exists(args.config):
        with open(args.config, 'r') as f:
            config_from_file = yaml.safe_load(f) or {}
            CONFIG.update(config_from_file)

    ssl_context = None
    if CONFIG["ssl_private_key"] and CONFIG["ssl_full_chain"]:
        ssl_context = (CONFIG["ssl_full_chain"], CONFIG["ssl_private_key"])

    print("\n-----Starting mock server-----\n")
    app.run(host='0.0.0.0', port=8443 if ssl_context else 8080, ssl_context=ssl_context)
