#!/bin/bash

# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

if [[ "$1" != "--standalone" ]]; then
    python3 -u setup_sample_data.py
    if [ $? -ne 0 ]; then
        exit 1
    fi
fi

exec python3 -u server.py