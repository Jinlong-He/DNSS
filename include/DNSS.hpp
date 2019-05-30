//
//  DNSS.hpp
//
//  Created by 何锦龙 on 2019/5/17.
//  Copyright © 2019年 何锦龙. All rights reserved.
//

#ifndef DNSS_hpp
#define DNSS_hpp
#include "Operation.hpp"
#include "Parse.hpp"

using namespace atm;

namespace dnss {
    typedef unordered_set<StillOperation*> StillOperations;
    typedef unordered_set<JumpOperation*> JumpOperations;
    typedef unordered_map<Symbol, StillOperations> StillTransMap;
    typedef unordered_map<Symbol, JumpOperations> JumpTransMap;
    typedef unordered_map<Symbol, ID> AddressMap;
    typedef unordered_map<Symbol, Symbols> Symbol2SymbolsMap;
    typedef unordered_map<JumpOperation*, Symbols> JumpOP2SymbolsMap;
    typedef vector<AddressMap> AddressMaps;
    typedef vector<GNFA*> NFARelation;
    typedef vector<NFARelation> NFARelations;
    typedef vector<Symbol2> Symbol2s;
    typedef unordered_set<GNFA*> GNFAs;
    typedef unordered_map<NFA<Symbol>*, DFA<Symbol>*> DFAMap;
    typedef unordered_map<DFA<Symbol>*, DFA<Symbol>*> ComMap;
    typedef unordered_map<DFA<Symbol>*, ID> CodeMap;
    typedef unordered_map<Activity*, Symbol> ActivityMap;
    typedef unordered_map<Aft, Symbol> AftMap;
    typedef vector<Activity*> ActivityVec;
    typedef vector<Aft> AftVec;
    typedef vector<PDS<Symbol>*> PDSMap;

    class DNSS {
    private:
        Symbol initialSymbol;
        Symbols alphabet;
        StillTransMap stillTransMap;
        JumpTransMap jumpTransMap;
        JumpOP2SymbolsMap jumpOP2SymbolsMap;
        JumpOperations jumpOperations;
        PDS<Symbol>* pds;
        Symbols ctpSymbols;
        SymbolsVec powerset;
        SymbolMap symbolMap;
        RecordSymbolVec recordSymbolMap;
        PDSStateMap pdsStateMap;
        Symbols recordAlphabet;
        NFARelations nfaRelations;
        NFARelations workRelations;
        NFARelations newWorkRelations;
        AddressMaps relationAddressMaps;
        AddressMaps workRelationAddressMaps;
        AddressMaps newWorkRelationAddressMaps;
        AddressMap addressMap;
        Symbol2SymbolsMap symbolsMap;
        GNFAs gnfas;
        DFA<Symbol>* res;
        DFAMap dfaMap;
        ComMap comMap;
        CodeMap codeMap;
        ActivityMap activityMap;
        ActivityVec activityVec;
        AftMap aftMap;
        AftVec aftVec;
        PDSMap pdsMap;
    public:
        DNSS() : pds(nullptr) {}

        DNSS(Symbol iSymbol, Symbols& symbols) : initialSymbol(iSymbol), alphabet(symbols), pds(nullptr) {}

        DNSS(Parse& parse) : pds(nullptr) {
            activityVec.push_back(nullptr);
            activityVec.push_back(nullptr);
            ID id = 0;
            for (Aft aft : parse.getAfts()) {
                aftMap[aft] = id++;
                aftVec.push_back(aft);
            }
            Symbol s = 2;
            for (Activity* act : parse.getActivities()) {
                alphabet.insert(s);
                addressMap[s] = aftMap[act -> getAft()];
                activityMap[act] = s++;
                activityVec.push_back(act);
            }
            initialSymbol = activityMap[parse.getMainActivity()];
            unordered_map<Symbol2, JumpOperation*> jumpOPMap;
            for (Action* action : parse.getActions()) {
                Activity* sAct = action -> getSourceAct();
                Activity* tAct = action -> getTargetAct();
                Symbol ss = activityMap[sAct];
                Symbol ts = activityMap[tAct];
                mkPOP(ts);
                if (!(action -> hasNTKFlag())) {
                    if (action -> hasCTKFlag()) {
                        mkCTK(ss, ts);
                    } else if (action -> hasCTPFlag()) {
                        mkCTP(ss, ts);
                    } else if (action -> hasSTPFlag()) {
                        if (ss != ts)
                            mkPUSH(ss, ts);
                    } else {
                        mkPUSH(ss, ts);
                    }
                } else {
                    if (action -> hasCTKFlag()) {
                        JumpOperation* op = jumpOPMap[Symbol2(0, ts)];
                        if (!op) {
                            op = new JumpCTK(ts);
                            jumpOPMap[Symbol2(0, ts)] = op;
                            jumpOperations.insert(op);
                        }
                        jumpTransMap[ss].insert(op);
                        jumpOP2SymbolsMap[op].insert(ss);
                    } else if (tAct -> getLmd() == lmd_stk || action -> hasCTPFlag()) {
                        ctpSymbols.insert(ts);
                        JumpOperation* op = jumpOPMap[Symbol2(1, ts)]; 
                        if (!op) { 
                            op = new JumpCTP(ts);
                            jumpOPMap[Symbol2(1, ts)] = op;
                            jumpOperations.insert(op);
                        }
                        jumpTransMap[ss].insert(op);
                        jumpOP2SymbolsMap[op].insert(ss);
                    } else if (action -> hasSTPFlag()) {
                        JumpOperation* op = jumpOPMap[Symbol2(3, ts)];
                        if (!op) {
                            op = new JumpPUSH(ts);
                            jumpOPMap[Symbol2(3, ts)] = op;
                            jumpOperations.insert(op);
                        }
                        jumpTransMap[ss].insert(op);
                        jumpOP2SymbolsMap[op].insert(ss);
                    } else {
                        JumpOperation* op = jumpOPMap[Symbol2(3, ts)];
                        if (!op) {
                            op = new JumpPUSH(ts);
                            jumpOPMap[Symbol2(3, ts)] = op;
                            jumpOperations.insert(op);
                        }
                        jumpTransMap[ss].insert(op);
                        jumpOP2SymbolsMap[op].insert(ss);
                    }
                }
            }
        }

        DNSS(Symbol iSymbol, Symbol2s symbol2s) : initialSymbol(iSymbol), pds(nullptr) {
            for (Symbol2 s2 : symbol2s) {
                alphabet.insert(s2.first);
                addressMap[s2.first] = s2.second;
            }
        }

        ~DNSS() {
            delete pds;
            for (auto& mapPair : stillTransMap) {
                for (StillOperation* op : mapPair.second) {
                    delete op;
                }
            }
            for (JumpOperation* op : jumpOperations) {
                delete op;
            }
            for (auto gnfa : gnfas) {
                delete gnfa;
            }
        }

        void mkPOP(Symbol sourceSymbol);
        void mkPUSH(Symbol sourceSymbol, Symbol targetSymbol);
        void mkCTP(Symbol sourceSymbol, Symbol targetSymbol);
        void mkCTK(Symbol sourceSymbol, Symbol targetSymbol);
        void mkJumpPUSH(Symbol sourceSymbol, Symbol targetSymbol);
        void mkJumpCTP(Symbol sourceSymbol, Symbol targetSymbol);
        void mkJumpCTK(Symbol sourceSymbol, Symbol targetSymbol);

        void mkPDS();

        void test();

        void addRelation(NFARelation& relation, AddressMap& relationAddressMap);

        void addRelationNew(NFARelation& relation, AddressMap& relationAddressMap);

        void updateRelation(NFARelation& relation, GNFA* gnfa, ID pos, NFARelation& newRelation, AddressMap& newRelationAddressMap);

        void updateRelation(NFARelation& relation, GNFA* gnfa, NFARelation& newRelation, AddressMap& newRelationAddressMap);

        GNFA* mkTopGNFA(GNFA* gnfa, Symbol topSymbol);

        GNFA* mkTopGNFA(GNFA* gnfa, Symbols topSymbols);

        void getNFA(const DFA<Symbol>& dfa);

        bool isNewRelation(NFARelation& relation, NFARelations& relations);

        bool isEqual(NFARelation& relation1, NFARelation& relation2);
    };
}

#endif /* DNSS_hpp */
