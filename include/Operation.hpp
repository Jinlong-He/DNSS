//
//  Operation.hpp
//
//  Created by 何锦龙 on 2019/5/17.
//  Copyright © 2019年 何锦龙. All rights reserved.
//

#ifndef Operation_hpp
#define Operation_hpp
#include "GNFA.hpp"
namespace dnss {

    class Operation {
    protected:
        Symbol targetSymbol;
    public:
        Operation() : targetSymbol(0) {}

        Operation(Symbol s) : targetSymbol(s) {}

        Symbol getTargetSymbol() {
            return targetSymbol;
        }
    };

    class JumpOperation : public Operation {
    public:
        JumpOperation() : Operation() {}

        JumpOperation(Symbol s) : Operation(s) {}

        virtual ~JumpOperation() {}

        virtual GNFA* mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap) = 0;

    };

    class StillOperation : public Operation {
    public:
        StillOperation() {}

        StillOperation(Symbol s) : Operation(s) {}

        virtual ~StillOperation() {}

        virtual void mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& stateMap) = 0;
    };

    class POP : public StillOperation {
    public:
        POP() : StillOperation() {}

        ~POP() {}

        void mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& stateMap);

    };

    class PUSH : public StillOperation {
    public:
        PUSH() : StillOperation() {}

        PUSH(Symbol s) : StillOperation(s) {}

        ~PUSH() {}

        void mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& stateMap);

    };

    class CTK: public StillOperation {
    public:
        CTK() : StillOperation() {}

        CTK(Symbol s) : StillOperation(s) {}

        ~CTK() {}

        void mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& stateMap);
    };

    class CTP: public StillOperation {
    public:
        CTP() : StillOperation() {}

        CTP(Symbol s) : StillOperation(s) {}

        ~CTP() {}

        void mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& stateMap);
    };

    class JumpPUSH : public JumpOperation {
    public:
        JumpPUSH() : JumpOperation() {}

        JumpPUSH(Symbol s) : JumpOperation(s) {}

        ~JumpPUSH() {}

        GNFA* mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap);

    };

    class JumpCTP : public JumpOperation {
    public:
        JumpCTP() : JumpOperation() {}

        JumpCTP(Symbol s) : JumpOperation(s) {}

        ~JumpCTP() {}

        GNFA* mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap);

    };

    class JumpCTK : public JumpOperation {
    public:
        JumpCTK() : JumpOperation() {}

        JumpCTK(Symbol s) : JumpOperation(s) {}

        ~JumpCTK() {}

        GNFA* mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap);

    };


}

#endif /* Operation_hpp */
