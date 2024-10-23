# ZKP-Based FAT16 File System

This project enhances the traditional FAT16 file system by integrating zero-knowledge proof (ZKP) techniques for secure file ownership, integrity verification, and privacy-preserving access control. It allows users to store encrypted files and verify their ownership and integrity without revealing the file's content. This system ensures robust data security, prevents unauthorized access, and counters tampering using cryptographic proofs.

## Features

- **File Encryption**: All files are encrypted using a symmetric encryption algorithm (AES). The user retains control of the encryption key.
- **Zero-Knowledge Proof (ZKP) Ownership**: Users can prove ownership of files without exposing sensitive keys or file content.
- **Integrity Verification**: File integrity is verified using cryptographic hashes (SHA-256). Users can prove that a file has not been tampered with using ZKP-based verification.
- **ZKP-Based Access Control**: Permission management is handled using ZKP to prove access rights without revealing credentials.
- **Tamper Resistance**: ZKP mechanisms ensure that any changes or unauthorized file modifications are detected without revealing the file content.
- **Compatibility**: The system is built on top of the FAT16 file system, ensuring compatibility with the original structure while adding cryptographic enhancements.

## Architecture Overview

1. Encryption Layer

   :

   - Before writing to the FAT16 file system, all file data is encrypted using AES encryption. The encryption key is held by the user and not stored in the file system.

2. Proof Generation

   :

   - When accessing a file, the user generates a zero-knowledge proof to show they have the decryption key without revealing the key itself.

3. Integrity Layer

   :

   - Each file’s hash is computed and stored. Users can prove file integrity without accessing the actual file data through a ZKP protocol.

4. Access Control

   :

   - Permissions to read/write files are checked using zero-knowledge proofs. Users prove they have access rights, verified by the system without revealing their credentials.

5. Tamper Detection

   :

   - Any modification to the encrypted file will result in a different hash. ZKP can verify the file’s original hash, allowing detection of tampering without accessing the file’s content.

## Requirements

- **FAT16 File System**: The system works on top of FAT16, so the base storage format is FAT16-compliant.

- Cryptographic Libraries

  :

  - AES encryption/decryption library.
  - SHA-256 hash function.
  - Zero-knowledge proof protocols (e.g., Schnorr, Bulletproofs).

## Usage

1. **Initializing the File System**:
   - Initialize the FAT16 file system if not already available. All files will be handled in an encrypted format on top of this.
2. **Adding Files**:
   - Files can be added to the system, and they will automatically be encrypted.
   - An integrity hash will be generated, and a ZKP proof will be created for ownership and integrity verification.
3. **Verifying Files**:
   - Users can verify that they own a file and prove its integrity without accessing the raw data.
4. **Accessing Files**:
   - When a user tries to access a file, they will generate a ZKP to verify their access rights. Once verified, the file is decrypted for the user.
