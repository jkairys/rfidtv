# Use the official Python 3.11 image as the base image
FROM python:3.11

# Set the working directory inside the container
WORKDIR /app

# Copy the requirements.txt file to the working directory
COPY . /app

# Install the Python dependencies
RUN pip install --no-cache-dir .

# Set the entry point for the container
CMD [ "rfidtv"]