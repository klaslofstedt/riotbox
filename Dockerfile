FROM --platform=linux/amd64 ubuntu:20.04

# Set the DEBIAN_FRONTEND environment variable to noninteractive
ENV DEBIAN_FRONTEND=noninteractive
# Install required packages for AWS Amplify and AWS CDK
RUN apt-get update && apt-get install -y apt-utils curl python3 python3-pip jq nano unzip
RUN curl -sL https://deb.nodesource.com/setup_18.x | bash -
RUN apt-get install -y nodejs
RUN apt-get install -y git
# Install required packages for esp-idf
RUN apt-get install -y wget flex bison gperf python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
# Install npm
RUN npm install -g npm@9.6.2
# Install AWS CLI version 2
RUN curl https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip -o awscliv2.zip \
    && unzip awscliv2.zip \
    && ./aws/install \
    && rm -rf aws awscliv2.zip
# Install AWS CDK for infrastructure & backend
RUN npm install -g aws-cdk
# Install AWS Amplify for mobile frontend
RUN npm install -g @aws-amplify/cli
# Clone esp-idf v5.0.1
RUN git clone --recursive -b v5.0.1 https://github.com/espressif/esp-idf.git
# Change the working directory to esp-idf
WORKDIR /esp-idf
RUN ./install.sh esp32
# Set required ESP-IDF environment variables
ENV IDF_PATH=/esp-idf
ENV PATH="/root/.espressif/tools/xtensa-esp32-elf/esp-2021r2-8.4.0/xtensa-esp32-elf/bin:${PATH}"
# Copy the source code into container
WORKDIR /riotbox
COPY ./app /riotbox/app
COPY ./secrets /riotbox/secrets
COPY ./scripts/docker /riotbox/scripts/docker
# Declare arguments
ARG AWS_ACCESS_KEY_ID
ARG AWS_SECRET_ACCESS_KEY_ID
ARG AWS_REGION
ARG PROJECT_NAME
ARG SECRETS_PASSWORD
# Set the arguments as environment variables so they are available to the scripts
ENV AWS_ACCESS_KEY_ID=${AWS_ACCESS_KEY_ID}
ENV AWS_SECRET_ACCESS_KEY_ID=${AWS_SECRET_ACCESS_KEY_ID}
ENV AWS_REGION=${AWS_REGION}
ENV PROJECT_NAME=${PROJECT_NAME}
ENV SECRETS_PASSWORD=${SECRETS_PASSWORD}
# Create an /root/.aws profile based on .env
WORKDIR /riotbox/scripts/docker/dockerfile/
RUN ./dockerfile_create_aws_profile.sh
# Set the working directory
WORKDIR /riotbox