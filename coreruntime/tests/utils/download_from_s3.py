# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import boto3
from botocore.exceptions import NoCredentialsError
from botocore.config import Config
from botocore import UNSIGNED
import os
import shutil
import argparse

def download_folder_from_s3(default_s3_bucket: str, s3_folder: str, output_dir: str, archive_output: bool):
    bucket_name = os.getenv("AWS_LLM_BUCKET_DEV", default_s3_bucket)
    if not bucket_name:
        raise EnvironmentError("Missing required environment variable: AWS_LLM_BUCKET_DEV")

    if os.path.exists(output_dir) and os.listdir(output_dir):
        print(f"Folder already exists and is not empty at: {output_dir}, skipping download.")
        return

    os.makedirs(output_dir, exist_ok=True)

    try:
        s3 = boto3.client("s3", config=Config(signature_version=UNSIGNED))
    except NoCredentialsError:
        raise EnvironmentError("AWS credentials not found. Please configure using `aws configure` or set env vars.")

    paginator = s3.get_paginator("list_objects_v2")
    pages = paginator.paginate(Bucket=bucket_name, Prefix=s3_folder)

    print(f"Downloading S3 folder: s3://{bucket_name}/{s3_folder}")

    for page in pages:
        for obj in page.get("Contents", []):
            key = obj["Key"]
            if key.endswith("/"):
                continue  # Skip folders

            rel_path = os.path.relpath(key, s3_folder)
            local_file_path = os.path.join(output_dir, rel_path)

            print(f"Downloading {key} → {local_file_path}")
            s3.download_file(bucket_name, key, local_file_path)

    print("Download complete.")

    if archive_output:
        shutil.make_archive(output_dir, "zip", output_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Download a folder from S3 to a local directory")
    parser.add_argument("--default_bucket", help="S3 bucket name")
    parser.add_argument("--prefix", required=True, help="S3 folder prefix (path inside the bucket)")
    parser.add_argument("--output", required=True, help="Local output folder (default: mockserver_assets)")
    parser.add_argument("--archive_output", required=True, help="Should the folder be stored as a zip file.")

    args = parser.parse_args()

    download_folder_from_s3(args.default_bucket, args.prefix, args.output, args.archive_output)
