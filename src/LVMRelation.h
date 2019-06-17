/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2019, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file LVMRelation.h
 *
 * Defines LVM Relations
 *
 ***********************************************************************/

#pragma once

#include "LVMIndex.h"
#include "ParallelUtils.h"
#include "RamIndexAnalysis.h"
#include "RamTypes.h"
#include "Relation.h"

#include <deque>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace souffle {

/**
 * A relation, composed of a collection of indexes.
 */
class LVMRelation {
public:
    /**
     * Creates a relation, build all necessary indexes.
     */
    LVMRelation(std::size_t arity, std::string& name, std::vector<std::string> attributeTypes,
            const MinIndexSelection& orderSet, IndexFactory factory = &createBTreeIndex);

    LVMRelation(LVMRelation& other) = delete;

    /**
     * Drops an index from the maintained indexes. All but one index
     * may be removed.
     */
    void removeIndex(const size_t& indexPos);

    /**
     * Add the given tuple to this relation.
     */
    bool insert(TupleRef tuple);

    /**
     * Add the given tuple to this relation.
     */
    bool insert(const RamDomain* tuple) {
        return insert(TupleRef(tuple, arity));
    }

    /**
     * Add all entries of the given relation to this relation.
     */
    void insert(const LVMRelation& other);

    /**
     * Tests whether this relation contains the given tuple.
     */
    bool contains(TupleRef tuple) const;

    /**
     * Obtains a stream to scan the entire relation.
     */
    Stream scan() const;

    /**
     * Obtains a stream covering the interval between the two given entries.
     */
    Stream range(const size_t& indexPos, TupleRef low, TupleRef high) const;

    /**
     * Removes the content of this relation, but retains the empty indexes.
     */
    void clear();

    /**
     * Swaps the content of this and the given relation, including the
     * installed indexes.
     */
    void swap(LVMRelation& other);
    
    /**
     * Set level
     */
    void setLevel(size_t level) {
        this->level = level;    //TODO necessary?
    }

    /**
     * Return the level of the relation.
     */
    size_t getLevel() const;

    /**
     * Return the relation name.
     */
    const std::string& getName() const;

    /**
     * Return the attribute types
     */
    const std::vector<std::string>& getAttributeTypeQualifiers() const;

    /**
     * Return arity
     */
    size_t getArity() const;

    /**
     * Return number of tuples in relation (full-order)
     */
    size_t size() const;

    /**
     * Check if the relation is empty
     */
    bool empty() const;

    /**
     * Clear all indexes
     */
    void purge();

    /**
     * Check if a tuple exists in realtion
     */
    bool exists(const TupleRef& tuple) const;

    /**
     * Extend another relation
     */
    void extend(const LVMRelation& rel);

protected:
    // Relation name
    std::string relName;

    // Relation Arity
    size_t arity;

    // Relation attributes types
    std::vector<std::string> attributeTypes;

    // a map of managed indexes
    std::vector<std::unique_ptr<Index>> indexes;

    // a pointer to the main index within the managed index
    Index* main;

    // relation level
    size_t level;
};


/**
 * Interpreter Equivalence Relation
 */

//class LVMEqRelation : public LVMIndirectRelation {
//public:
//    LVMEqRelation(size_t relArity, const MinIndexSelection* orderSet, std::string relName,
//            std::vector<std::string>& attributeTypes)
//            : LVMIndirectRelation(relArity, orderSet, relName, attributeTypes) {}
//
//    /** Insert tuple */
//    void insert(const RamDomain* tuple) override {
//        // TODO: (pnappa) an eqrel check here is all that appears to be needed for implicit additions
//        // TODO: future optimisation would require this as a member datatype
//        // brave soul required to pass this quest
//        // // specialisation for eqrel defs
//        // std::unique_ptr<binaryrelation> eqreltuples;
//        // in addition, it requires insert functions to insert into that, and functions
//        // which allow reading of stored values must be changed to accommodate.
//        // e.g. insert =>  eqRelTuples->insert(tuple[0], tuple[1]);
//
//        // for now, we just have a naive & extremely slow version, otherwise known as a O(n^2) insertion
//        // ):
//
//        for (auto* newTuple : extend(tuple)) {
//            LVMIndirectRelation::insert(newTuple);
//            delete[] newTuple;
//        }
//    }
//
//    /** Find the new knowledge generated by inserting a tuple */
//    std::vector<RamDomain*> extend(const RamDomain* tuple) override {
//        std::vector<RamDomain*> newTuples;
//
//        newTuples.push_back(new RamDomain[2]{tuple[0], tuple[0]});
//        newTuples.push_back(new RamDomain[2]{tuple[0], tuple[1]});
//        newTuples.push_back(new RamDomain[2]{tuple[1], tuple[0]});
//        newTuples.push_back(new RamDomain[2]{tuple[1], tuple[1]});
//
//        std::vector<const RamDomain*> relevantStored;
//        for (const RamDomain* vals : *this) {
//            if (vals[0] == tuple[0] || vals[0] == tuple[1] || vals[1] == tuple[0] || vals[1] == tuple[1]) {
//                relevantStored.push_back(vals);
//            }
//        }
//
//        for (const auto vals : relevantStored) {
//            newTuples.push_back(new RamDomain[2]{vals[0], tuple[0]});
//            newTuples.push_back(new RamDomain[2]{vals[0], tuple[1]});
//            newTuples.push_back(new RamDomain[2]{vals[1], tuple[0]});
//            newTuples.push_back(new RamDomain[2]{vals[1], tuple[1]});
//            newTuples.push_back(new RamDomain[2]{tuple[0], vals[0]});
//            newTuples.push_back(new RamDomain[2]{tuple[0], vals[1]});
//            newTuples.push_back(new RamDomain[2]{tuple[1], vals[0]});
//            newTuples.push_back(new RamDomain[2]{tuple[1], vals[1]});
//        }
//
//        return newTuples;
//    }
//    /** Extend this relation with new knowledge generated by inserting all tuples from a relation */
//    void extend(const LVMRelation& rel) override {
//        std::vector<RamDomain*> newTuples;
//        // store all values that will be implicitly relevant to the those that we will insert
//        for (const auto* tuple : rel) {
//            for (auto* newTuple : extend(tuple)) {
//                newTuples.push_back(newTuple);
//            }
//        }
//        for (const auto* newTuple : newTuples) {
//            LVMIndirectRelation::insert(newTuple);
//            delete[] newTuple;
//        }
//    }
//};

}  // end of namespace souffle
