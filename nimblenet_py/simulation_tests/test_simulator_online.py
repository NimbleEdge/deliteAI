# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from deliteai import simulator
import time
import numpy as np

def test_simulator():

    config = '''
                {
                    "deviceId": "deviceId",
                    "clientId": "testclient",
                    "host": "http://localhost:8080",
                    "clientSecret": "dummy",
                    "debug": true,
                    "online" : true,
                    "compatibilityTag": "MODEL_ADDITION"
                }
            '''
    # initialize nimblenet
    init = simulator.initialize(config)
    assert init == True

    while not simulator.is_ready():
        time.sleep(1)

    output = simulator.run_method("add_and_multiply", {"num": np.asarray([5])})
    assert output["output"] == 14


if __name__ == "__main__":
    test_simulator()
