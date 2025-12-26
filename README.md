## Secure AES-128 OTA Update System for ESP8266
This project implements a **secure, production-grade Over-The-Air (OTA) firmware update mechanism** for the **ESP8266**. It mitigates common OTA security risks by integrating **AES-128 encryption** for confidentiality and **SHA-256 hashing** for integrity verification.

---

## Features
- **AES-128-CBC Encryption:**  Firmware is encrypted both **at rest** and **during transit**.
- **SHA-256 Integrity Verification:**  Ensures firmware has not been corrupted or tampered with.
- **Streaming Decryption:**  Optimized for ESP8266 memory constraints — decrypts firmware in **16-byte blocks**.
- **Rollback Safety:**  EEPROM flags detect boot failures and prevent device bricking.
- **Version Control:**  Metadata-driven logic ensures **only newer firmware versions** are installed.

---

## Tech Stack

### Embedded
- **Hardware**: ESP8266 
- **Framework**: Arduino
- **Libraries**:
  - `ESP8266HTTPClient`
  - `AESLib`
  - `SHA256`

### Backend
- **Language**: Python 3
- **Framework**: Flask (Server)
- **Crypto**: PyCryptodome (Encryption)

---

## Project Structure
- `firmware.ino`: ESP8266 source code.
- `encrypt.py`: Python script to pad, hash, and encrypt the binary.
- `server.py`: Flask server to host the `.enc` files.

---

## How It Works

* **Encryption (Server Side):** `encrypt.py` takes a raw `.bin`, adds PKCS#7 padding, calculates a SHA256 hash, and encrypts it with a 16-byte Secret Key and a random IV.
* **Metadata:** A secure metadata file is created containing the new version number and the expected hash.
* **Check:** The ESP8266 polls the `/meta` endpoint. It decrypts the metadata and compares the version to its own.
* **Update:** If a newer version exists, it downloads the encrypted firmware, decrypts it block-by-block, writes to flash, and verifies the hash before rebooting.

---

## Getting Started
### 1. Requirements
    pip install pycryptodome flask

### 2. Prepare the ESP8266
* Update the `ssid` and `password` in `firmware.ino`.
* Note the `CURRENT_VERSION` (e.g., `33`).
* Upload the code to your ESP8266.
* Ensure the `KEY` in `encrypt.py` matches the `key` in `firmware.ino`.

### 3. Generate an Update
* Change the code in Arduino IDE to a new version (e.g., `34`).
* Go to **Sketch > Export Compiled Binary.**
* Run the encryption script:
  ```bash
  python encrypt.py

### 4. Start the Server
    python server.py
Reset the ESP8266 and watch the Serial Monitor for the decryption and update progress.

---

## Security Features Explained
#### Streaming Decryption
To avoid `Memory Allocation Failed` errors, this project does not load the entire encrypted file into RAM. It reads, decrypts, and writes to flash in small chunks, making it highly stable.

#### IV Chaining
For every firmware update, a random 16-byte Initialization Vector (IV) is generated. This ensures that even if the same firmware is uploaded twice, the encrypted output will be different, preventing pattern-matching attacks.

#### Integrity Verification
The SHA256 hash is calculated on the padded firmware. The ESP8266 re-calculates this hash during the decryption process. If even a single bit is wrong, the update is aborted before the device reboots.

---

## License
This project is licensed under the MIT License - see the LICENSE file for details.
