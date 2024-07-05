// Copyright (c) 2023 The Navio developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NAVIO_BLSCT_SET_MEM_PROOF_SET_MEM_PROVER_H
#define NAVIO_BLSCT_SET_MEM_PROOF_SET_MEM_PROVER_H

#include <vector>
#include <blsct/arith/elements.h>
#include <blsct/range_proof/generators.h>
#include <blsct/set_mem_proof/set_mem_proof_setup.h>
#include <blsct/set_mem_proof/set_mem_proof.h>
#include <blsct/building_block/imp_inner_prod_arg.h>
#include <blsct/common.h>
#include <hash.h>

template <typename T>
class SetMemProofProver {
public:
    using Scalar = typename T::Scalar;
    using Point = typename T::Point;
    using Scalars = Elements<Scalar>;
    using Points = Elements<Point>;

    static SetMemProof<T> Prove(
        const SetMemProofSetup<T>& setup,
        const Points& Ys_src, // N Pedersen Commitment Y^n
        const Point& sigma,  // Commitment of the set member
        const Scalar& m,  // Message used for the commitment of the set member
        const Scalar& f,  // Mask f used for the commitment of the set member
        const Scalar& eta_fiat_shamir, // entropy for fiat shamir
        const blsct::Message& eta_phi // entropy for building generators
    );

    static bool Verify(
        const SetMemProofSetup<T>& setup,
        const Points& Ys_src,
        const Scalar& eta_fiat_shamir,
        const blsct::Message& eta_phi,
        const SetMemProof<T>& proof  // Output of Prove()
    );

#ifndef BOOST_UNIT_TEST
private:
#endif
    static HashWriter GenInitialFiatShamir(
        const Points& Ys,
        const Point& A1,
        const Point& A2,
        const Point& S1,
        const Point& S2,
        const Point& S3,
        const Point& phi,
        const Scalar& eta
    );

    static Scalar ComputeX(
        const SetMemProofSetup<T>& setup,
        const Scalar& omega,
        const Scalar& y,
        const Scalar& z,
        const Point& T1,
        const Point& T2
    );

    static Points ExtendYs(
        const SetMemProofSetup<T>& setup,
        const Points& Ys_src,
        const size_t& new_size
    );

    static const Scalar& One();
};

#endif // NAVIO_BLSCT_SET_MEM_PROOF_SET_MEM_PROVER_H
