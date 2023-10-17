#!/bin/bash

bash -i -c "source /esp-idf/export.sh && cd $APP_ESP32_PATH && idf.py build"
