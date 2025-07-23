<!-- DL_DOCS::WEBSITE_BLOCK_START
```{eval-rst}
:orphan:
```
DL_DOCS::WEBSITE_BLOCK_END -->

# Mock Server

## Set up

1. Download assets from S3

    ```bash
    python3 coreruntime/tests/utils/download_from_s3.py \
        --default_bucket deliteai \
        --prefix build-dependencies/llama-3.2-1B/onnx \
        --output mockserver/mockserver_assets/llama-3 \
        --archive_output True
    ```

2. Start services using docker compose:

    ```bash
    cd mockserver
    docker compose up --build -d
    ```

3. To stop the mockserver:

    ```bash
    docker compose down -v
    ```

### Points to note

* The above command runs an instance of Postgres, Mock Model Distribution Service (MDS),
  Mock Dashboard Management Services (DMS), Ingestion service and mock proxy server.
* Mock server runs on `http://localhost:8080/` by default.
* Ingestion service runs on `http://localhost:8081/` by default. Update the API_KEY environment
  variable in docker-compose file to change the authentication key for ingestion service.
* Logger service runs on `http://localhost:8082/` by default. Update the API_KEY environment variable
  in docker-compose file to change the authentication key for logger service.
* By default some sample data is populated in `testclient` for `DEFAULT-TAG`.
* All data is managed locally in the containers. No data is persisted across runs.
* Always use `--build` flag to avoid using cached version of mock server.
* Using `-d` makes the services run in detached mode in the background.

### Monitoring

* To view state of pods run `docker compose ps`.
* To tail container logs use `docker compose logs -f service`. Here service can be `mock_server`,
  `mds`, `dms` or `postgres`.
* Cloud config for testclient is configured to push logs and metrics to the logger service. They can
  be viewed at `sdk_logs/metrics.log` and `sdk_logs/logs.log`. Unauthenticated and dropped events
  can be viewed in `sdk_logs/unauthenticated.log` and `sdk_logs/dropped.log`.
* Ingestion service writes the events to `ingestion_events/vector.log` file.
* For testing with android simulator set `LOGGER_URL` in `docker-compose.yaml` to `http://10.0.2.2:8082/v2`.

## Mock Server Standalone

Mock server can be run in standalone mode to point to an existing backend as proxy

1. Update `REMOTE_MDS_URL` and `REMOTE_DMS_URL` in `docker-compose-standalone.yaml`.
2. Start/stop service using docker compose:

    ```bash
    cd mockserver
    docker compose -f docker-compose-standalone.yaml up --build -d
    docker compose -f docker-compose-standalone.yaml down -v
    ```

## Mock Server configuration

1. Create a mock response

    ```text
    POST /mocker/expectation
    Body:
    {
        "path": "/abc/test", # required
        "response_delay": 2000, # in ms
        "status_code": 404, # if not provided, will just add response_delay to the proxied request
        "body": {},
        "headers": {},
        "repeat_count": 4 # mock this response for 4 times and then forget the mock and start forwarding
    }
    ```

2. Delete a mock response

    ```text
    DELETE /mocker/expectation
    Body:
    {
        "path": "/abc/test" # required
    }
    ```

3. List mock responses

    ```text
    GET /mocker/expectations
    ```

4. Delete all mock responses

    ```text
    POST /mocker/reset
    ```
