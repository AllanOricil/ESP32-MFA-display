#!/bin/bash

set -e

password=""
salt=""
while [[ $# -gt 0 ]]; do
  case $1 in
    --password)
      password=$2
      shift 2
      ;;
    --salt)
      salt=$2
      shift 2
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

if [ -z "$password" ]; then
  echo "Error: The --password parameter is required."
  echo "Usage: $0 --password <password>"
  echo "Example: $0 --password qw"
  exit 1
fi

if [ -z "$salt" ]; then
  echo "Error: The --salt parameter is required. It must be Base32 encoded"
  echo "Usage: $0 --salt <salt>"
  echo "Example: $0 --salt KXOFAZCLCES66ZW22ZWQVE2YMY======"
  exit 1
fi

ITERATIONS=10000
KEY_SIZE=32

DECODED_SALT=$(echo "$salt" | base32 --decode)
GENERATED_KEY=$(openssl kdf -keylen $KEY_SIZE -kdfopt digest:SHA256 -kdfopt pass:$password -kdfopt salt:"$DECODED_SALT" -kdfopt iter:$ITERATIONS PBKDF2)
echo "$GENERATED_KEY"
