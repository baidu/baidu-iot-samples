/*
* Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/* This file contains cert trust chain needed to communicate with Baidu (IoT Hub) */

#include "certs.h"

const char certificates[] =
// add trust external ca root
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFdjCCBF6gAwIBAgIRAK/f0g4MMudXFc/j/XKWz/cwDQYJKoZIhvcNAQELBQAw\r\n"
"gZAxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\r\n"
"BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMTYwNAYD\r\n"
"VQQDEy1DT01PRE8gUlNBIERvbWFpbiBWYWxpZGF0aW9uIFNlY3VyZSBTZXJ2ZXIg\r\n"
"Q0EwHhcNMTcwNzA2MDAwMDAwWhcNMTgwNzA2MjM1OTU5WjBnMSEwHwYDVQQLExhE\r\n"
"b21haW4gQ29udHJvbCBWYWxpZGF0ZWQxHTAbBgNVBAsTFFBvc2l0aXZlU1NMIFdp\r\n"
"bGRjYXJkMSMwIQYDVQQDDBoqLm1xdHQuaW90LmJqLmJhaWR1YmNlLmNvbTCCASIw\r\n"
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANKdgMr24rlwfjd8hQLlnWDV4zne\r\n"
"5WG7awTQz+UaXQO3Cr2p0PzpkBcLIm8X0wM4aiY0t1rJOrgYSDfW5lzb4QNeZyWa\r\n"
"kwjZxOLrxYMpJ4ftiFIUZ525K3B5N1IKA/mrKKWYYoXp8Nt438K4ZSlVWdPYKIbQ\r\n"
"V1qCO/9nokzj093rgmdgDd9PZTd3Kd4tAkxPi3/SYz8bMfOW72tXLvJruMLUdaEJ\r\n"
"Ocv4zZOlZKCwR8gFp8s3hXjr3n2MnJrrocgRK/kvyAypuc3Hk5ELSpyzdTCEKTWO\r\n"
"f4tsOQlFVQH78pz2BMphpXCpMp5GIerocUBNnWg0ZYycTRh//GTOwCQuI6cCAwEA\r\n"
"AaOCAfEwggHtMB8GA1UdIwQYMBaAFJCvajqUWgvYkOoSVnPfQ7Q6KNrnMB0GA1Ud\r\n"
"DgQWBBSLtkNw1PlwT8nse1amajInCZgmcTAOBgNVHQ8BAf8EBAMCBaAwDAYDVR0T\r\n"
"AQH/BAIwADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwTwYDVR0gBEgw\r\n"
"RjA6BgsrBgEEAbIxAQICBzArMCkGCCsGAQUFBwIBFh1odHRwczovL3NlY3VyZS5j\r\n"
"b21vZG8uY29tL0NQUzAIBgZngQwBAgEwVAYDVR0fBE0wSzBJoEegRYZDaHR0cDov\r\n"
"L2NybC5jb21vZG9jYS5jb20vQ09NT0RPUlNBRG9tYWluVmFsaWRhdGlvblNlY3Vy\r\n"
"ZVNlcnZlckNBLmNybDCBhQYIKwYBBQUHAQEEeTB3ME8GCCsGAQUFBzAChkNodHRw\r\n"
"Oi8vY3J0LmNvbW9kb2NhLmNvbS9DT01PRE9SU0FEb21haW5WYWxpZGF0aW9uU2Vj\r\n"
"dXJlU2VydmVyQ0EuY3J0MCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5jb21vZG9j\r\n"
"YS5jb20wPwYDVR0RBDgwNoIaKi5tcXR0LmlvdC5iai5iYWlkdWJjZS5jb22CGG1x\r\n"
"dHQuaW90LmJqLmJhaWR1YmNlLmNvbTANBgkqhkiG9w0BAQsFAAOCAQEAiPtenflg\r\n"
"6vtqQW2dC3rH8UjwHjweE7O/7BtlBN7T9SXf6oOtJpkCqOUx/aKMgoQe6Mpu++QN\r\n"
"cD5dQVPkw7WEeFldgca3Y+pJW2fnbRgE1ZT7X6K2Jx2Y+k69KY7QOq4pxh5+i18n\r\n"
"koSZWHnHYYVCoyRNLIeJYkwgXyc0Kr6QsEzNBJ1Sw2NPpI0DviHbxctD3c/b5A6t\r\n"
"yd1FF3lsgQAvYWjMhGtW5ipfKI+Ors307hOQWSbCI1xW92bTzNJrBsNCXMm+RCJi\r\n"
"Ysy3M+40wLSXmPv8LDXRB0q78hQfPGr5vGVC8BeYINmITyEUTI7gLAD1QJcegdiU\r\n"
"mbmKazpwQnI2Ew==\r\n"
"-----END CERTIFICATE-----\r\n"
;
