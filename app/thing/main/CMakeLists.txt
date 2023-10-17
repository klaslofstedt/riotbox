idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(
    SRCS 
        "app/app.c"
        "app/ota.c" 
        "app/provision.c"
        "app/deploy.c"
        "app/mobile.c"
        "app/thing.c"
        "app/types/type.c"
        "app/types/default.c" 
        "app/types/switch.c" 
        "middlewares/wifi.c"
        "middlewares/mqtt.c" 
        "middlewares/ble.c"
        "middlewares/auth.c"
        "drivers/storage.c"
        "utilities/state.c"
        "utilities/event.c"
        "utilities/misc.c"
        "utilities/aes.c"
        "utilities/auth_aws_provision.c"
        "${project_dir}/../../secrets/decrypted/esp_auth/auth_aws_ota_thing_cert.c" 
        "${project_dir}/../../secrets/decrypted/esp_auth/auth_aws_ota_thing_key.c" 
        "${project_dir}/../../secrets/decrypted/esp_auth/auth_aws_ota_root_ca.c"
    INCLUDE_DIRS 
        "."
        "${project_dir}/../../secrets/decrypted/esp_auth"
    EMBED_TXTFILES 
        ${project_dir}/../../secrets/decrypted/esp_encryption/rsa_key.pem
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=unused-const-variable)

create_esp_enc_img(
    ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    ${project_dir}/../../secrets/decrypted/esp_encryption/rsa_key.pem 
    ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_secure.bin 
    app
)

