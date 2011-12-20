#!/bin/sh
openssl s_client -connect localhost:6000 -CAfile keys/client.pem
