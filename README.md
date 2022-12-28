# Gicame
**Generic Inter-process Communication And Message Exchange**

## What is Gicame
Gicame is a C++17 **cross-platform** library created to expose simple and modern interfaces for building applications that would normally be difficult and composed by OS-dependent code.
It wants to be:
 - as simple as possible,
 - fully accessible via C++ constructs and
 - compatible with all major operating systems without the need for modifications.

It focuses of distributed applications which need to communicate by exchanging messages or sharing data, but it also works as the core for many extensions whose purpose is to extend the C++ language with new and modern functionality.<br />
Gicame is build around the idea of **building blocks**: you only need few lines of code to create your application based on Gicame because the developer is offered a series of objects to be composed at will in order to give shape to their custom functionality.<br />
However, the library is still **under development** (alpha stage), for example, support for UDP connections has not yet been implemented.

## Build on POSIX Systems (GNU-Linux / FreeBSD / Mac OS X / Android / Cygwin)
There is a makefile! :D You already know what to do.<br />
Of all the POSIX systems, only GNU-Linux is officially supported, but spurious compilation tests are performed on the other systems as well.
For a list of tested compilers and systems, see the appropriate paragraph (*Compatibility and tests*).


## Build on Windows
You have three options:
- Install **Cygwin** and follow the instructions in the appropriate paragraph.
- Use **MSVC2022** to open the appropriate project file.
<br/>
For a list of tested compilers and systems, see the appropriate paragraph (*Compatibility and tests*).

## Compatibility and tests
This software is and will be tested only on 64bit systems.<br />
It "should" also run on 32bit systems, but no tests are performed.<br />
Some tests on earlier versions were successfully run on:
- **Windows (AMD64)**
	- Windows 10 (build 2004) using *MSVC2019*
	- Windows 10 (build 21H1) using *MSVC2019*
	- Windows 10 (build 21H1) using *MSVC2022*
	- Cygwin on Windows 10 (build 2004) using *gcc-g++ 10.2.0-1*
- **GNU/Linux (AMD64)**
	- Ubuntu 18.04 LTS using *g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0*
- **FreeBSD (AMD64)**
	- FreeBSD 12.1 using *g++ (FreeBSD Ports Collection) 9.3.0*
- **Mac OS X (AMD64)**
    - Mac OS X Catalina 10.15 using *Xcode 11.3.1*
- **Android 10 (Aarch64)**
    - Android 10 using *clang version 10.0.1 on Termux*

# Gicame-crypto
Gicame-crypto is a C++17 **cross-platform** Gicame's extension that is a front-end library for OpenSSL.<br />
It is designed to offer OpenSSL functionalities while maintaining the same ideas as Gicame, such as ease of use.<br />
In the future, Gicame may adopt other cryptographic methods, but for now all of the functions are wrappers for OpenSSL.<br />
### Example: digital signature
This will require only 2 lines of code! Just load your PEM private key ad use it to sign your content!
```
const EvpKey privKey = EvpKey::fromPrivateKeyPemFile("../../../examples/KeyPairExample/privKey.pem");
const std::vector<byte_t> digitalSign = Signer::sign(privKey, fileContent.data(), fileContent.size());
```
### Example: AES-128 encryption
Select your EncryptionAlgorithm, read the file, choose your SymmetricKey and your iv. Encryption requires few lines of code.
```
const EncryptionAlgorithm ea = EncryptionAlgorithm::AES_128_CBC;
const SymmetricKey key = SymmetricKey::createSymmetricKey(keyBuffer, sizeof(keyBuffer), ea);
const std::vector<byte_t> fileContent = Gicame::IO::readFileContent(filePath);
const std::vector<byte_t> encrypted = Encryptor::encrypt(key, ea, iv, fileContent.data(), fileContent.size());
```
### Example: HMAC of a very large file
When you cannot load the entire file in memory (or you may not have the whole data content at all), it is still possible to compute HMACs (or HASHes, or chipthertexts, etc...) chunk by chunk.
```
HmacCalculator hmacCalc(SymmetricKey::createSymmetricKey(...), HashAlgorithm::SHA256);
hmacCalc.update(chunk0, size0);
/* ... */
hmacCalc.update(chunkN, sizeN);
std::vector<byte_t> hmac = hmacCalc.finalize();
```

# License
See the *LICENSE* file. Basically everything is licensed under LGPL v2.1.
## Gicame-crypto and OpenSSL integration
Gicame-crypto makes use of OpenSSL licensed under the double OpenSSL/SSLeay license.<br/>
*This product includes cryptographic software written by Eric Young (eay@cryptsoft.com).*<br/>
*This product includes software written by Tim Hudson (tjh@cryptsoft.com).*
