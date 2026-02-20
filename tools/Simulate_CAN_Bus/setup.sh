#!/usr/bin/env bash

set -e  # Exit immediately if a command fails

ENV_NAME="environment"

echo "Creating virtual environment..."
python3 -m venv $ENV_NAME

echo "Activating environment..."
source $ENV_NAME/bin/activate

echo "Upgrading pip..."
python -m pip install --upgrade pip

echo "Installing requirements..."
pip install -r requirements.txt

echo ""
echo "======================================"
echo " Environment setup complete."
echo " To activate later, run:"
echo " source $ENV_NAME/bin/activate"
echo "======================================"
