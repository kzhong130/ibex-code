/**
 * @file paillier.c
 *
 * @date Created on: Aug 25, 2012
 * @author Camille Vuillaume
 * @copyright Camille Vuillaume, 2012
 *
 * This file is part of Paillier-GMP.
 *
 * Paillier-GMP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Paillier-GMP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Paillier-GMP.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include "paillier.h"
#include "tools.h"

/** Function L(u)=(u-1)/n
 *
 * @ingroup Paillier
 * @param[out] result output result (u-1)/n
 * @param[in] input u
 * @param[in] ninv input n^{-1} mod 2^len
 * @param[in] len input bit length
 * @return 0 if no error
 *
 * The function L is evaluated using the pre-computed value n^{-1} mod 2^len.
 * The calculation a/n is computed as a*n^{-1} mod 2^len
 * - First a non-modular multiplication with n^{-1} mod 2^len is calculated.
 * - Then the result is reduced by masking higher bits.
 */
int paillier_ell(mpz_t result, mpz_t input, mpz_t ninv, mp_bitcnt_t len) {
	mpz_t mask;

	mpz_init(mask);

	mpz_sub_ui(result, input, 1);
	mpz_mul(result, result, ninv);
	mpz_setbit(mask, len);
	mpz_sub_ui(mask, mask, 1);
	mpz_and(result, result, mask);

	mpz_clear(mask);
	return 0;
}

/**
 * The function does the following.
 * - It generates two (probable) primes p and q having bits/2 bits.
 * - It computes the modulus n=p*q and sets the basis g to 1+n.
 * - It pre-computes n^{-1} mod 2^len.
 * - It pre-computes the CRT paramter p^{-2} mod q^2.
 * - It calculates lambda = lcm((p-1)*(q-1))
 * - It calculates mu = L(g^lambda mod n^2)^{-1} mod n using the CRT.
 * .
 * Since /dev/random is one of the sources of randomness in prime generation, the program may block.
 * In that case, you have to wait or move your mouse to feed /dev/random with fresh randomness.
 */
int paillier_keygen(paillier_public_key *pub, paillier_private_key *priv, mp_bitcnt_t len) {
	mpz_t p, q, n2, temp, mask, g;

	mpz_init(p);
	mpz_init(q);
	mpz_init(n2);
	mpz_init(temp);
	mpz_init(mask);
	mpz_init(g);

	//write bit lengths
	priv->len = len;
	pub->len = len;

	//generate p and q
	gen_prime(p, len/2);
	gen_prime(q, len/2);

	//calculate modulus n=p*q
	mpz_mul(pub->n, p, q);
	mpz_mul(priv->n, p, q);

	//set g = 1+n
	mpz_add_ui(g, pub->n, 1);

	//compute n^{-1} mod 2^{len}
	mpz_setbit(temp, len);
	if(!mpz_invert(priv->ninv, pub->n, temp)) {
		fputs("Inverse does not exist!\n", stderr);
		mpz_clear(p);
		mpz_clear(q);
		mpz_clear(n2);
		mpz_clear(temp);
		mpz_clear(mask);
		mpz_clear(g);
		exit(1);
	}

	//compute p^2 and q^2
	mpz_mul(priv->p2, p, p);
	mpz_mul(priv->q2, q, q);

	//generate CRT parameter
	mpz_invert(priv->p2invq2, priv->p2, priv->q2);

	//calculate lambda = lcm(p-1,q-1)
	mpz_clrbit(p, 0);
	mpz_clrbit(q, 0);
	mpz_lcm(priv->lambda, p, q);

	//calculate n^2
	mpz_mul(n2, pub->n, pub->n);

	//calculate mu
	crt_exponentiation(temp, g, priv->lambda, priv->lambda, priv->p2invq2, priv->p2, priv->q2);

	paillier_ell(temp, temp, priv->ninv, len);

	if(!mpz_invert(priv->mu, temp, pub->n)) {
		fputs("Inverse does not exist!\n", stderr);
		mpz_clear(p);
		mpz_clear(q);
		mpz_clear(n2);
		mpz_clear(temp);
		mpz_clear(mask);
		mpz_clear(g);
		exit(1);
	}

	//free memory and exit
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(n2);
	mpz_clear(temp);
	mpz_clear(mask);
	mpz_clear(g);
	return 0;
}

/**
 * The function calculates c=g^m*r^n mod n^2 with r random number.
 * Encryption benefits from the fact that g=1+n, because (1+n)^m = 1+n*m mod n^2.
 */
int paillier_encrypt(mpz_t ciphertext, mpz_t plaintext, paillier_public_key *pub) {
	mpz_t n2, r;

	if(mpz_cmp(pub->n, plaintext)) {
		mpz_init(n2);
		mpz_init(r);

		//re-compute n^2
		mpz_mul(n2, pub->n, pub->n);

		//generate random r and reduce modulo n
		gen_pseudorandom(r, pub->len);
		mpz_mod(r, r, pub->n);
		if(mpz_cmp_ui(r, 0) == 0) {
			fputs("random number is zero!\n", stderr);
			mpz_clear(n2);
			mpz_clear(r);
			exit(1);
		}

		//compute r^n mod n2
		mpz_powm(ciphertext, r, pub->n, n2);

		//compute (1+m*n)
		mpz_mul(r, plaintext, pub->n);
		mpz_add_ui(r, r, 1);

		//multiply with (1+m*n)
		mpz_mul(ciphertext, ciphertext, r);
		mpz_mod(ciphertext, ciphertext, n2);

		mpz_clear(n2);
		mpz_clear(r);
	}
	return 0;
}

/**
 * The decryption function computes m = L(c^lambda mod n^2)*mu mod n.
 * The exponentiation is calculated using the CRT, and exponentiations mod p^2 and q^2 run in their own thread.
 *
 */
int paillier_decrypt(mpz_t plaintext, mpz_t ciphertext, paillier_private_key *priv) {
	//compute exponentiation c^lambda mod n^2
	crt_exponentiation(plaintext, ciphertext, priv->lambda, priv->lambda, priv->p2invq2, priv->p2, priv->q2);

	//compute L(c^lambda mod n^2)
	paillier_ell(plaintext, plaintext, priv->ninv, priv->len);

	//compute L(c^lambda mod n^2)*mu mod n
	mpz_mul(plaintext, plaintext, priv->mu);
	mpz_mod(plaintext, plaintext, priv->n);

	return 0;
}

/**
 * "Add" two plaintexts homomorphically by multiplying ciphertexts modulo n^2.
 * For example, given the ciphertexts c1 and c2, encryptions of plaintexts m1 and m2,
 * the value c3=c1*c2 mod n^2 is a ciphertext that decrypts to m1+m2 mod n.
 */
int paillier_homomorphic_add(mpz_t ciphertext3, mpz_t ciphertext1, mpz_t ciphertext2, paillier_public_key *pub) {
	mpz_t n2;

	mpz_init(n2);
	mpz_mul(n2, pub->n, pub->n);

	mpz_mul(ciphertext3, ciphertext1, ciphertext2);
	mpz_mod(ciphertext3, ciphertext3, n2);

	mpz_clear(n2);
	return 0;
}

/**
 * "Multiplies" a plaintext with a constant homomorphically by exponentiating the ciphertext modulo n^2 with the constant as exponent.
 * For example, given the ciphertext c, encryptions of plaintext m, and the constant 5,
 * the value c3=c^5 n^2 is a ciphertext that decrypts to 5*m mod n.
 */
int paillier_homomorphic_multc(mpz_t ciphertext2, mpz_t ciphertext1, mpz_t constant, paillier_public_key *pub) {
	mpz_t n2;

	mpz_init(n2);
	mpz_mul(n2, pub->n, pub->n);
	mpz_powm(ciphertext2, ciphertext1, constant, n2);

	mpz_clear(n2);
	return 0;
}

int paillier_keygen_str(FILE *public_key, FILE *private_key, int len) {
	int result;
	paillier_public_key pub;
	paillier_private_key priv;

	paillier_public_init(&pub);
	paillier_private_init(&priv);

	//generate keys
	result = paillier_keygen(&pub, &priv, len);

	//export public key
	result |= paillier_public_out_str(public_key, &pub);

	//export private key
	result |= paillier_private_out_str(private_key, &priv);

	paillier_public_clear(&pub);
	paillier_private_clear(&priv);

	return result;
}

/**
 * Wrapper to the encryption function using stdio streams as inputs and output.
 * @see paillier_encrypt
 */
int paillier_encrypt_str(FILE *ciphertext, FILE *plaintext, FILE *public_key) {
	mpz_t c, m;
	int result;
	paillier_public_key pub;

	mpz_init(c);
	mpz_init(m);
	paillier_public_init(&pub);

	//import public key
	paillier_public_in_str(&pub, public_key);

	//convert plaintext from stream
	gmp_fscanf(plaintext, "%Zx\n", m);
	if(mpz_cmp(m, pub.n) >= 0) {
		fputs("Warning, plaintext is larger than modulus n!\n", stderr);
	}

	//calculate encryption
	result = paillier_encrypt(c, m, &pub);

	//convert ciphertext to stream
	gmp_fprintf(ciphertext, "%Zx\n", c);

	mpz_clear(c);
	mpz_clear(m);
	paillier_public_clear(&pub);

	return result;
}

/**
 * Wrapper to the decryption function using stdio streams as inputs and output.
 * @see paillier_decrypt
 */
int paillier_decrypt_str(FILE *plaintext, FILE *ciphertext, FILE *private_key) {
	mpz_t c, m, n2;
	paillier_private_key priv;
	int result;

	mpz_init(c);
	mpz_init(m);
	mpz_init(n2);
	paillier_private_init(&priv);

	//import private key
	paillier_private_in_str(&priv, private_key);

	//compute n^2
	mpz_mul(n2, priv.n, priv.n);

	//convert ciphertext from stream
	gmp_fscanf(ciphertext, "%Zx\n", c);
	if(mpz_cmp(c, n2) >= 0) {
		fputs("Warning, ciphertext is larger than modulus n^2!\n", stderr);
	}
	//calculate decryption
	result = paillier_decrypt(m, c, &priv);

	//convert plaintext to stream
	gmp_fprintf(plaintext, "%Zx\n", m);

	mpz_clear(c);
	mpz_clear(m);
	mpz_clear(n2);
	paillier_private_clear(&priv);

	return result;
}

/**
 * Wrapper to the homomorphic addition function using stdio streams as inputs and output.
 * @see paillier_homomorphic_add
 */
int paillier_homomorphic_add_str(FILE *ciphertext3, FILE *ciphertext1, FILE *ciphertext2, FILE *public_key) {
	mpz_t c3, c1, c2, n2;
	paillier_public_key pub;
	int result;

	mpz_init(c3);
	mpz_init(c1);
	mpz_init(c2);
	mpz_init(n2);
	paillier_public_init(&pub);

	//import public key
	paillier_public_in_str(&pub, public_key);

	//compute n^2
	mpz_mul(n2, pub.n, pub.n);

	//convert ciphertexts from stream
	gmp_fscanf(ciphertext1, "%Zx\n", c1);
	if(mpz_cmp(c1, n2) >= 0) {
		fputs("Warning, first ciphertext is larger than modulus n^2!\n", stderr);
	}
	gmp_fscanf(ciphertext2, "%Zx\n", c2);
	if(mpz_cmp(c2, n2) >= 0) {
		fputs("Warning, second ciphertext is larger than modulus n^2!\n", stderr);
	}
	//calculate decryption
	result = paillier_homomorphic_add(c3, c1, c2, &pub);

	//convert result to stream
	gmp_fprintf(ciphertext3, "%Zx\n", c3);

	mpz_clear(c3);
	mpz_clear(c1);
	mpz_clear(c2);
	mpz_clear(n2);
	paillier_public_clear(&pub);

	return result;
}

/**
 * Wrapper to the homomorphic multiplication function using stdio streams as inputs and output.
 * @see paillier_homomorphic_add
 */
int paillier_homomorphic_multc_str(FILE *ciphertext2, FILE *ciphertext1, FILE *constant, FILE *public_key) {
	mpz_t c2, c1, k, n2;
	paillier_public_key pub;
	int result;

	mpz_init(c2);
	mpz_init(c1);
	mpz_init(k);
	mpz_init(n2);
	paillier_public_init(&pub);

	//import public key
	paillier_public_in_str(&pub, public_key);

	//compute n^2
	mpz_mul(n2, pub.n, pub.n);

	//convert ciphertext from stream
	gmp_fscanf(ciphertext1, "%Zx\n", c1);
	if(mpz_cmp(c1, n2) >= 0) {
		fputs("Warning, first ciphertext is larger than modulus n^2!\n", stderr);
	}
	gmp_fscanf(constant, "%Zx\n", k);
	if(mpz_cmp(k, pub.n) >= 0) {
		fputs("Warning, constant is larger than modulus n!\n", stderr);
	}
	//calculate decryption
	result = paillier_homomorphic_multc(c2, c1, k, &pub);

	//convert result to stream
	gmp_fprintf(ciphertext2, "%Zx\n", c2);

	mpz_clear(c2);
	mpz_clear(c1);
	mpz_clear(k);
	mpz_clear(n2);
	paillier_public_clear(&pub);

	return result;
}

void paillier_public_init(paillier_public_key *pub) {
	mpz_init(pub->n);
}

void paillier_private_init(paillier_private_key *priv) {
	mpz_init(priv->lambda);
	mpz_init(priv->mu);
	mpz_init(priv->p2);
	mpz_init(priv->q2);
	mpz_init(priv->p2invq2);
	mpz_init(priv->ninv);
	mpz_init(priv->n);
}

void paillier_public_clear(paillier_public_key *pub) {
	mpz_clear(pub->n);
}

void paillier_private_clear(paillier_private_key *priv) {
	mpz_clear(priv->lambda);
	mpz_clear(priv->mu);
	mpz_clear(priv->p2);
	mpz_clear(priv->q2);
	mpz_clear(priv->p2invq2);
	mpz_clear(priv->ninv);
	mpz_clear(priv->n);
}

int paillier_public_out_str(FILE *fp, paillier_public_key *pub) {
	int printf_ret, result = 0;

	printf_ret = gmp_fprintf(fp, "%d\n", pub->len);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;

	printf_ret = gmp_fprintf(fp, "%Zx\n", pub->n);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;

	return result;
}

int paillier_private_out_str(FILE *fp, paillier_private_key *priv) {
	int printf_ret, result = 0;

	printf_ret = gmp_fprintf(fp, "%d\n", priv->len);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->lambda);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->mu);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->p2);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->q2);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->p2invq2);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->ninv);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;
	printf_ret = gmp_fprintf(fp, "%Zx\n", priv->n);
	if(printf_ret < 0) return printf_ret;
	result += printf_ret;

	return result;
}

int paillier_public_in_str(paillier_public_key *pub, FILE *fp) {
	int scanf_ret, result = 0;

	scanf_ret = gmp_fscanf(fp, "%d\n", &(pub->len));
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", pub->n);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	return result;
}

int paillier_private_in_str(paillier_private_key *priv, FILE *fp) {
	int scanf_ret, result = 0;

	scanf_ret = gmp_fscanf(fp, "%d\n", &(priv->len));
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->lambda);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->mu);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->p2);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->q2);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->p2invq2);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->ninv);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	scanf_ret = gmp_fscanf(fp, "%Zx\n", priv->n);
	if(scanf_ret < 0) return scanf_ret;
	result += scanf_ret;

	return result;
}