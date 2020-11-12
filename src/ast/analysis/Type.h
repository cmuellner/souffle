/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Type.h
 *
 * A collection of type analyses operating on AST constructs.
 *
 ***********************************************************************/

#pragma once

#include "AggregateOp.h"
#include "FunctorOps.h"
#include "ast/NumericConstant.h"
#include "ast/analysis/Analysis.h"
#include "ast/analysis/TypeSystem.h"
#include "souffle/BinaryConstraintOps.h"
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace souffle::ast {
class Argument;
class Aggregator;
class BinaryConstraint;
class Clause;
class Functor;
class FunctorDeclaration;
class IntrinsicFunctor;
class NumericConstant;
class UserDefinedFunctor;
}  // namespace souffle::ast

namespace souffle::ast::analysis {

class TypeAnalysis : public Analysis {
public:
    static constexpr const char* name = "type-analysis";

    TypeAnalysis() : Analysis(name) {}

    void run(const TranslationUnit& translationUnit) override;

    void print(std::ostream& os) const override;

    /** Get the computed types for the given argument. */
    TypeSet const& getTypes(const Argument* argument) const {
        return argumentTypes.at(argument);
    }

    /**
     * Analyse the given clause and computes for each contained argument
     * a set of potential types. If the set associated to an argument is empty,
     * no consistent typing can be found and the rule can not be properly typed.
     *
     * @return a map mapping each contained argument to a set of types
     */
    static std::map<const Argument*, TypeSet> analyseTypes(
            const TranslationUnit& tu, const Clause& clause, std::ostream* logs = nullptr);

    // Checks whether an argument has been assigned a valid type
    bool hasValidTypeInfo(const Argument* argument) const;

    std::set<TypeAttribute> getTypeAttributes(const Argument* arg) const;

    /** -- Functor-related methods -- */
    IntrinsicFunctors getValidIntrinsicFunctorOverloads(const IntrinsicFunctor& inf) const;
    TypeAttribute getFunctorReturnType(const Functor* functor) const;
    TypeAttribute getFunctorArgType(const Functor* functor, const size_t idx) const;
    const std::vector<TypeAttribute>& getFunctorArgTypes(const UserDefinedFunctor& udf) const;

    bool isStatefulFunctor(const UserDefinedFunctor* udf) const;
    static bool isMultiResultFunctor(const Functor& functor);

    /** -- Polymorphism-related methods -- */
    NumericConstant::Type getPolymorphicNumericConstantType(const NumericConstant* nc) const;
    const std::map<const NumericConstant*, NumericConstant::Type>& getNumericConstantTypes() const;
    AggregateOp getPolymorphicOperator(const Aggregator* agg) const;
    BinaryConstraintOp getPolymorphicOperator(const BinaryConstraint* bc) const;
    FunctorOp getPolymorphicOperator(const IntrinsicFunctor* inf) const;

private:
    // General type analysis
    std::map<const Argument*, TypeSet> argumentTypes;
    VecOwn<Clause> annotatedClauses;
    std::stringstream analysisLogs;

    /* Return a new clause with type-annotated variables */
    static Own<Clause> createAnnotatedClause(
            const Clause* clause, const std::map<const Argument*, TypeSet> argumentTypes);

    // Polymorphic objects analysis
    std::map<const IntrinsicFunctor*, const IntrinsicFunctorInfo*> functorInfo;
    std::map<std::string, const FunctorDeclaration*> udfDeclaration;
    std::map<const NumericConstant*, NumericConstant::Type> numericConstantType;
    std::map<const Aggregator*, AggregateOp> aggregatorType;
    std::map<const BinaryConstraint*, BinaryConstraintOp> constraintType;

    bool analyseIntrinsicFunctors(const TranslationUnit& translationUnit);
    bool analyseNumericConstants(const TranslationUnit& translationUnit);
    bool analyseAggregators(const TranslationUnit& translationUnit);
    bool analyseBinaryConstraints(const TranslationUnit& translationUnit);

    bool isFloat(const Argument* argument) const;
    bool isUnsigned(const Argument* argument) const;
    bool isSymbol(const Argument* argument) const;
};

}  // namespace souffle::ast::analysis
