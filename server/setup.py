from setuptools import setup, find_packages

with open("requirements.txt") as f:
    requirements = f.read().splitlines()

setup(
    name="rfidtv",
    version="1.0.0",
    description="A Python app for RFID TV",
    author="Your Name",
    author_email="your@email.com",
    packages=find_packages(),
    install_requires=requirements,
    entry_points={
        "console_scripts": [
            "rfidtv = rfidtv.main:main",
        ],
    },
)
