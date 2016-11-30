#pragma once

#include <libscapi/include/CryptoInfra/Protocol.hpp>
#include "../../primitives/CommunicationConfig.hpp"
#include "../../primitives/CryptoPrimitives.hpp"
#include "../../primitives/CircuitInput.hpp"
#include "../primitives/BucketBundle.hpp"
#include "../primitives/EvaluationPackage.hpp"
#include "../../common/KeyUtils.hpp"

/**
* This class represents the first party in the online phase of Malicious Yao protocol. <P>
*
* The full protocol specification is described in "Blazing Fast 2PC in the "Offline/Online Setting with Security for
* Malicious Adversaries" paper by Yehuda Lindell and Ben Riva, page 20 - FIGURE E.3 (The Online Stage).
*
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Asaf Cohen)
*
*/
class OnlineProtocolP1 : public Protocol, public Malicious{
	
private:
	shared_ptr<CommParty> channel;				//Used to communicate between the parties.

	/*
	* The buckets to use in the online protocol.
	* The buckets contain some circuits to evaluate.
	*/
	BucketBundle mainBucket;	//Contain the main circuits (for ex. AES).		
	BucketBundle crBucket;	//Contain the cheating recovery circuits.		

	shared_ptr<CircuitInput> input;					//Input for the main circuits.	
	vector<vector<byte>> maskedX;					//The xor of the placement mask and the input for the circuit.
	SecretKey proofOfCheating;			//Generated by p1 and sent to p2 in order to detect cheating.

	/**
	* Computes the main circuits part.
	*
	* pseudo code:
	* 1. Receive the requested input from p2
	* 2. Send the commitment masks
	* 3. Decommit of p2 input wires' keys
	* 4. Send the xor of the placement mask with the input
	* 5. Decommit of p1 input wires' keys
	* 6. Select and encrypt proof to send to p2
	*/
	void evaluateMainCircuit();

	/**
	* Computes the cheating recovery circuits part.
	*
	* pseudo code:
	* 1. Receive the requested input from p2
	* 2. Send the commitment masks
	* 3. Decommit on p2 input wires' keys
	* 4. Send the xor of the placement mask with the input
	* 5. Decommit on p1 input wires' keys
	* 6. Decommit on both output wires' keys
	* @throws IOException
	*/
	void evaluateCheatingRecoveryCircuit();

	/**
	* Receive the input bit of p2 input wires.
	* @return the received bits.
	* @throws IOException IN case of problem during the receiving.
	*/
	CircuitInput receiveY2();

	/**
	* Adds to the evaluationPackage the commitment masks of every circuit in the bucket.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	*/
	void sendCommitmentMasks(BucketBundle & bucket, EvaluationPackage & evaluationPackage);

	/**
	* Adds the decommitments of all Y2 inputs of every circuit in the bucket according to the given y2 input bits.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	* @param y2 Input bit for each p2 input wire.
	*/
	void decommitY2InputKeys(BucketBundle & bucket, EvaluationPackage & evaluationPackage, CircuitInput & y2);

	/**
	* Adds the decommitments of all D2 inputs of every circuit in the bucket according to the given d2 input bits.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	* @param d2 Input bit for each p2 input wire.
	*/
	void decommitD2InputKeys(BucketBundle & bucket, EvaluationPackage & evaluationPackage, CircuitInput & d2);

	/**
	* Adds to the evaluationPackage the placement masks of every circuit in the bucket.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	*/
	void sendPlacementMasks(BucketBundle & bucket, EvaluationPackage & evaluationPackage);

	/**
	* Adds the decommitments of p1 inputs of every circuit in the bucket.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	*/
	void sendXInputKeys(BucketBundle & bucket, EvaluationPackage & evaluationPackage);

	/**
	* Xor each output wire with random values in order to encrypt the output keys.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	* @throws InvalidInputException
	*/
	void selectAndXorProof(BucketBundle & bucket, EvaluationPackage & evaluationPackage);

	/**
	* Adds the decommitments of the output keys of every circuit in the bucket.
	* @param bucket The bucket to use in the protocol.
	* @param evaluationPackage The message that will be sent to p2.
	*/
	void decommitOutputKeys(BucketBundle & bucket, EvaluationPackage & evaluationPackage);


public:
	/**
	* A constructor that sets the given parameters and initializes some inline members.
	* @param mainExecution Contains some parameters regarding the execution of the main circuits.
	* @param crExecution Contains some parameters regarding the execution of the cheating recovery circuits.
	* @param primitives Contains some primitives object to use during the protocol.
	* @param communication Configuration of communication between parties.
	* @param mainBucket Contain the main circuits (for ex. AES).
	* @param crBucket Contain the cheating recovery circuits.
	*/
	OnlineProtocolP1(CommunicationConfig & communication, BucketBundle & mainBucket, BucketBundle & crBucket);

	void setInput(shared_ptr<CircuitInput> protocolInput) { input = protocolInput; }

	/**
	* Executes the first side of the online protocol.<p>
	* basically, it computes the main circuit and than the cheating recovery circuit.
	*/
	void run() override;
	
};

