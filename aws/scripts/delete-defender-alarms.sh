#!/bin/bash

# shellcheck disable=SC2086

AUDIT_ROLE_NAME="iotSt2021DefenderAuditRole"

AUDIT_CHECK_NAME="st202CertCheck"
SNS_TOPIC_NAME="st2021-iot-defender-results"
SNS_PUBLISH_ROLE_NAME="st2021iotDefenderSnsPublishRole"
SNS_PUBLISH_ROLE_POLICY="st2021iotDefenderSnsPublishRolePolicy"
IOT_SECURITY_PROFILE_NAME="St2021RegisteredDevicesIssues"
CURRENT_ACCOUNT=$(aws sts get-caller-identity | jq -r '.Account')
CURRENT_REGION=$(aws configure get region)

aws iam detach-role-policy --role-name $AUDIT_ROLE_NAME --policy-arn arn:aws:iam::aws:policy/service-role/AWSIoTDeviceDefenderAudit
aws iam detach-role-policy --role-name $SNS_PUBLISH_ROLE_NAME --policy-arn "arn:aws:iam::$CURRENT_ACCOUNT:policy/$SNS_PUBLISH_ROLE_POLICY"

aws iam delete-policy --policy-arn "arn:aws:iam::$CURRENT_ACCOUNT:policy/$SNS_PUBLISH_ROLE_POLICY"

aws iam delete-role --role-name $AUDIT_ROLE_NAME
aws iam delete-role --role-name $SNS_PUBLISH_ROLE_NAME


aws sns delete-topic --topic-arn "arn:aws:sns:$CURRENT_REGION:$CURRENT_ACCOUNT:$SNS_TOPIC_NAME"

aws iot delete-security-profile --security-profile-name $IOT_SECURITY_PROFILE_NAME

aws iot delete-scheduled-audit --scheduled-audit-name $AUDIT_CHECK_NAME