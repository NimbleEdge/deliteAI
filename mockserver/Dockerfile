# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

FROM python:3.10-slim
WORKDIR /app
COPY mockserver_assets/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY mockserver_assets .
RUN chmod +x entrypoint.sh
CMD ["./entrypoint.sh"]