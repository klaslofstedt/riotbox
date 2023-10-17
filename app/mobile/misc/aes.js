import { randomBytes } from 'react-native-randombytes';
import aesjs from 'aes-js';

export const aesCrypto = (inputString, keyString) => {
    // Convert string to bytes as it's half the size to string
    const keyBytes = aesjs.utils.hex.toBytes(keyString);
    const inputBytes = aesjs.utils.utf8.toBytes(inputString);
    // Create a random 16 byte IV
    const iv = randomBytes(16);
    const counter = new aesjs.Counter(iv);
    const aesCtr = new aesjs.ModeOfOperation.ctr(keyBytes, counter);
    const encryptedBytes = aesCtr.encrypt(inputBytes);

    return { encryptedBytes, iv };
};
