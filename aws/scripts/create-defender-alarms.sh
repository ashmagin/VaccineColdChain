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

SNS_TOPIC_ARN=$(aws sns create-topic --name $SNS_TOPIC_NAME --attributes DisplayName="IotDefender results" | jq -r '.TopicArn')

SNS_ROLE_ARN=$(aws iam create-role \
  --role-name ${SNS_PUBLISH_ROLE_NAME} \
   --assume-role-policy-document "{\"Version\":\"2012-10-17\",\"Statement\":[{\"Effect\":\"Allow\",\"Principal\":{\"Service\":\"iot.amazonaws.com\"},\"Action\":\"sts:AssumeRole\"}]}" | jq -r '.Role.Arn' )

SNS_PUBLISH_ROLE_POLICY_ARN=$(aws iam create-policy  --policy-name $SNS_PUBLISH_ROLE_POLICY --policy-document "{\"Version\":\"2012-10-17\",\"Statement\":[{\"Effect\":\"Allow\",\"Action\":[\"sns:Publish\"],\"Resource\":[\"${SNS_TOPIC_ARN}\"]}]}" | jq -r '.Policy.Arn')

aws iam attach-role-policy \
  --policy-arn $SNS_PUBLISH_ROLE_POLICY_ARN \
  --role-name ${SNS_PUBLISH_ROLE_NAME}

aws iam create-role \
  --role-name ${AUDIT_ROLE_NAME} \
   --assume-role-policy-document "{\"Version\":\"2012-10-17\",\"Statement\":[{\"Effect\":\"Allow\",\"Principal\":{\"Service\":\"iot.amazonaws.com\"},\"Action\":\"sts:AssumeRole\"}]}"

AUDIT_ROLE_ARN=$(aws iam get-role --role-name ${AUDIT_ROLE_NAME} | jq -r '.Role.Arn')

aws iam attach-role-policy \
  --policy-arn arn:aws:iam::aws:policy/service-role/AWSIoTDeviceDefenderAudit \
   --role-name ${AUDIT_ROLE_NAME}

aws iot update-account-audit-configuration \
  --role-arn $AUDIT_ROLE_ARN \
  --audit-notification-target-configurations "{\"SNS\":{\"targetArn\":\"${SNS_TOPIC_ARN}\",\"roleArn\":\"${SNS_ROLE_ARN}\",\"enabled\":true}}" \
  --audit-check-configurations "{\"CA_CERTIFICATE_EXPIRING_CHECK\":{\"enabled\":true},\"CA_CERTIFICATE_KEY_QUALITY_CHECK\":{\"enabled\":true},\"CONFLICTING_CLIENT_IDS_CHECK\":{\"enabled\":true},\"DEVICE_CERTIFICATE_EXPIRING_CHECK\":{\"enabled\":true},\"DEVICE_CERTIFICATE_KEY_QUALITY_CHECK\":{\"enabled\":true},\"DEVICE_CERTIFICATE_SHARED_CHECK\":{\"enabled\":true},\"IOT_POLICY_OVERLY_PERMISSIVE_CHECK\":{\"enabled\":true},\"IOT_ROLE_ALIAS_ALLOWS_ACCESS_TO_UNUSED_SERVICES_CHECK\":{\"enabled\":true},\"LOGGING_DISABLED_CHECK\":{\"enabled\":true},\"REVOKED_CA_CERTIFICATE_STILL_ACTIVE_CHECK\":{\"enabled\":true},\"REVOKED_DEVICE_CERTIFICATE_STILL_ACTIVE_CHECK\":{\"enabled\":true},\"IOT_ROLE_ALIAS_OVERLY_PERMISSIVE_CHECK\":{\"enabled\":true}}"


aws iot create-scheduled-audit \
  --scheduled-audit-name $AUDIT_CHECK_NAME \
  --frequency DAILY \
  --target-check-names CA_CERTIFICATE_EXPIRING_CHECK CA_CERTIFICATE_KEY_QUALITY_CHECK CONFLICTING_CLIENT_IDS_CHECK DEVICE_CERTIFICATE_EXPIRING_CHECK DEVICE_CERTIFICATE_KEY_QUALITY_CHECK DEVICE_CERTIFICATE_SHARED_CHECK IOT_POLICY_OVERLY_PERMISSIVE_CHECK REVOKED_CA_CERTIFICATE_STILL_ACTIVE_CHECK REVOKED_DEVICE_CERTIFICATE_STILL_ACTIVE_CHECK


aws iot create-security-profile \
    --security-profile-name $IOT_SECURITY_PROFILE_NAME \
    --alert-targets "{\"SNS\":{\"alertTargetArn\":\"${SNS_TOPIC_ARN}\",\"roleArn\":\"${SNS_ROLE_ARN}\"}}" \
    --behaviors "[{\"name\":\"Authorization_failures_ML_behavior\",\"metric\":\"aws:num-authorization-failures\",\"criteria\":{\"consecutiveDatapointsToAlarm\":2,\"consecutiveDatapointsToClear\":2,\"mlDetectionConfig\":{\"confidenceLevel\":\"HIGH\"}},\"suppressAlerts\":true},{\"name\":\"Connection_attempts_ML_behavior\",\"metric\":\"aws:num-connection-attempts\",\"criteria\":{\"consecutiveDatapointsToAlarm\":2,\"consecutiveDatapointsToClear\":2,\"mlDetectionConfig\":{\"confidenceLevel\":\"HIGH\"}},\"suppressAlerts\":true},{\"name\":\"Disconnects_ML_behavior\",\"metric\":\"aws:num-disconnects\",\"criteria\":{\"consecutiveDatapointsToAlarm\":2,\"consecutiveDatapointsToClear\":2,\"mlDetectionConfig\":{\"confidenceLevel\":\"HIGH\"}},\"suppressAlerts\":true},{\"name\":\"Message_size_ML_behavior\",\"metric\":\"aws:message-byte-size\",\"criteria\":{\"consecutiveDatapointsToAlarm\":2,\"consecutiveDatapointsToClear\":2,\"mlDetectionConfig\":{\"confidenceLevel\":\"HIGH\"}},\"suppressAlerts\":true},{\"name\":\"Messages_received_ML_behavior\",\"metric\":\"aws:num-messages-received\",\"criteria\":{\"consecutiveDatapointsToAlarm\":2,\"consecutiveDatapointsToClear\":2,\"mlDetectionConfig\":{\"confidenceLevel\":\"HIGH\"}},\"suppressAlerts\":true},{\"name\":\"Messages_sent_ML_behavior\",\"metric\":\"aws:num-messages-sent\",\"criteria\":{\"consecutiveDatapointsToAlarm\":2,\"consecutiveDatapointsToClear\":2,\"mlDetectionConfig\":{\"confidenceLevel\":\"HIGH\"}},\"suppressAlerts\":true}]"

aws iot attach-security-profile \
    --security-profile-name $IOT_SECURITY_PROFILE_NAME \
    --security-profile-target-arn "arn:aws:iot:${CURRENT_REGION}:${CURRENT_ACCOUNT}:all/registered-things"