#include "DNSS.hpp"

namespace dnss {
    void DNSS::mkPOP(Symbol sourceSymbol) {
        stillTransMap[sourceSymbol].insert(new POP());
    }

    void DNSS::mkPUSH(Symbol sourceSymbol, Symbol targetSymbol) {
        stillTransMap[sourceSymbol].insert(new PUSH(targetSymbol));
    }

    void DNSS::mkCTP(Symbol sourceSymbol, Symbol targetSymbol) {
        stillTransMap[sourceSymbol].insert(new CTP(targetSymbol));
        ctpSymbols.insert(targetSymbol);
    }

    void DNSS::mkCTK(Symbol sourceSymbol, Symbol targetSymbol) {
        stillTransMap[sourceSymbol].insert(new CTK(targetSymbol));
    }

    void DNSS::mkJumpPUSH(Symbol sourceSymbol, Symbol targetSymbol) {
        jumpTransMap[sourceSymbol].insert(new JumpPUSH(targetSymbol));
    }

    void DNSS::mkJumpCTP(Symbol sourceSymbol, Symbol targetSymbol) {
        jumpTransMap[sourceSymbol].insert(new JumpCTP(targetSymbol));
        ctpSymbols.insert(targetSymbol);
    }

    void DNSS::mkJumpCTK(Symbol sourceSymbol, Symbol targetSymbol) {
        jumpTransMap[sourceSymbol].insert(new JumpCTK(targetSymbol));
    }

    void DNSS::mkPDS() {
        Utility::Combination(ctpSymbols, powerset);
        powerset.push_back(Symbols());
        pds = new PDS<Symbol>();
        pdsStateMap[eps] = pds -> mkState();
        pdsStateMap[bot] = pds -> mkState();
        pdsStateMap[p] = pds -> mkState();
        RecordSymbol botSymbol(bot, Symbols());
        RecordSymbol epsSymbol(eps, Symbols());
        symbolMap[botSymbol] = bot;
        symbolMap[epsSymbol] = eps;
        recordSymbolMap.push_back(epsSymbol);
        recordSymbolMap.push_back(botSymbol);
        recordAlphabet.insert(bot);
        Symbol s = 2;
        for (Symbol symbol : alphabet) {
            for (ID i = 0; i < powerset.size(); i++) {
                RecordSymbol recordSymbol(symbol, powerset[i]);
                recordAlphabet.insert(s);
                symbolsMap[symbol].insert(s);
                symbolMap[recordSymbol] = s++;
                recordSymbolMap.push_back(recordSymbol);
            }
        }
        for (auto& mapPair : stillTransMap) {
            for (StillOperation* stillOperation : mapPair.second) {
                stillOperation -> mkPDSTransition(mapPair.first, alphabet, ctpSymbols, powerset, symbolMap, pds, pdsStateMap);
            }
        }
        for (ID add = 0; add < aftVec.size(); add++) {
            PDS<Symbol>* newPDS = new PDS<Symbol>(*pds);
            for (auto& mapPair : jumpTransMap) {
                for (JumpOperation* op: mapPair.second) {
                    if (addressMap[op -> getTargetSymbol()] == add) {
                        op -> mkPDSTransition(mapPair.first, alphabet, ctpSymbols, powerset, symbolMap, newPDS, pdsStateMap);
                    }
                }
            }
            newPDS -> output();
            cout << endl;
            pdsMap.push_back(newPDS);
        }
    }

    void DNSS::test() {
        mkPDS();
        pds -> output();
        cout << endl;
        GNFA* gnfa = GNFA::mkGNFA(initialSymbol, initialSymbol, recordAlphabet, ctpSymbols, symbolMap, pdsStateMap);
        gnfa -> postStar(pds, recordSymbolMap);
        gnfas.insert(gnfa);
        NFARelation relation;
        AddressMap relationAddressMap;
        relation.push_back(gnfa);
        relationAddressMap[addressMap[initialSymbol]] = 0;
        nfaRelations.push_back(relation);
        workRelations.push_back(relation);
        relationAddressMaps.push_back(relationAddressMap);
        workRelationAddressMaps.push_back(relationAddressMap);
        FAs fas;
        ID step = 0;
        while (workRelations.size() > 0) {
            for (ID i = 0; i < workRelations.size(); i++) {
                cout << "step " << step++ << endl;
                addRelationNew(workRelations[i], workRelationAddressMaps[i]);
            }
            workRelations.clear();
            workRelationAddressMaps.clear();
            if (newWorkRelations.size() > 0) {
                workRelations.insert(workRelations.end(), newWorkRelations.begin(), newWorkRelations.end());
                workRelationAddressMaps.insert(workRelationAddressMaps.end(), newWorkRelationAddressMaps.begin(), newWorkRelationAddressMaps.end());
                newWorkRelations.clear();
                newWorkRelationAddressMaps.clear();
            }
        }
        cout << "finish calculating..." << endl;
        for (ID i = 0; i < nfaRelations.size(); i++) {
            FAList faList;
            for (GNFA* gnfa : nfaRelations[i]) {
                NFA<Symbol>* nfa = gnfa -> getNFA();
                DFA<Symbol>* dfa = dfaMap[nfa];
                if (!dfa) {
                    dfa = &(nfa -> minimize());
                    dfaMap[nfa] = dfa;
                }
                faList.push_back(dfa);
            }
            fas.insert(&(FA<Symbol>::concatenateFA(faList)));
        }
        getNFA(FA<Symbol>::unionFA(fas));
    }

    void DNSS::getNFA(const DFA<Symbol>& dfa) {
        NFA<Symbol> nfa(dfa);
        for (NFAState<Symbol>* state : nfa.getStates()) {
            NFATransMap transMap; 
            for (auto& mapPair : state -> getTransMap()) {
                Symbol symbol = recordSymbolMap[mapPair.first].first;
                transMap[symbol].insert(mapPair.second.begin(), mapPair.second.end());
            }
            state -> getTransMap().clear();
            state -> getTransMap() = transMap;
        }
        res = &(nfa.minimize());
        res -> print("res.dot");
        for (ID i = 2; i < activityVec.size(); i++)
            cout << i << " " << activityVec[i] -> getName() << endl;
    }

    GNFA* DNSS::mkTopGNFA(GNFA* gnfa, Symbol topSymbol) {
        NFAState2Map copyMap;
        GNFA* newGNFA = GNFA::cpGNFA(gnfa, copyMap);
        NFA<Symbol>* nfa = gnfa -> getNFA();
        NFA<Symbol>* newNFA = newGNFA -> getNFA();
        NFAStates work;
        NFAState<Symbol>* initialState = nfa -> getInitialState();
        NFAState<Symbol>* newInitialState = newNFA -> getInitialState();
        for (auto& mapPair : initialState -> getTransMap()) {
            Symbol symbol = mapPair.first;
            if (symbolsMap[topSymbol].count(symbol) > 0) {
                for (NFAState<Symbol>* targetState : mapPair.second) {
                    newInitialState -> addTrans(symbol, copyMap[targetState]);
                    if (targetState == initialState) continue;
                    work.insert(targetState);
                }
            }
        }
        GNFA::cpNFA(copyMap, work);
        return newGNFA;
    }

    GNFA* DNSS::mkTopGNFA(GNFA* gnfa, Symbols topSymbols) {
        NFAState2Map copyMap;
        GNFA* newGNFA = GNFA::cpGNFA(gnfa, copyMap);
        NFA<Symbol>* nfa = gnfa -> getNFA();
        NFA<Symbol>* newNFA = newGNFA -> getNFA();
        NFAStates work;
        NFAState<Symbol>* initialState = nfa -> getInitialState();
        NFAState<Symbol>* newInitialState = newNFA -> getInitialState();
        for (auto& mapPair : initialState -> getTransMap()) {
            Symbol symbol = mapPair.first;
            if (topSymbols.count(recordSymbolMap[symbol].first) > 0) {
                for (NFAState<Symbol>* targetState : mapPair.second) {
                    newInitialState -> addTrans(symbol, copyMap[targetState]);
                    if (targetState == initialState) continue;
                    work.insert(targetState);
                }
            }
        }
        GNFA::cpNFA(copyMap, work);
        return newGNFA;
    }

    ID getCode(DFA<Symbol>* dfa) {
        DFAState<Symbol>* initialState = dfa -> getInitialState();
        ID code = hash<double>()((initialState -> getTransMap().size()) * 3.14);
        for (auto& mapPair : initialState -> getTransMap()) {
            code ^= mapPair.first;
        }
        return code;
    }

    bool DNSS::isEqual(NFARelation& relation1, NFARelation& relation2) {
        //cout << "Equal in" << endl;
        if (relation1.size() != relation2.size()) {
            //cout << "Equal out0" << endl;
            return false;
        }
        for (ID i = 0; i < relation1.size(); i++) {
            GNFA* gnfa1 = relation1[i];
            GNFA* gnfa2 = relation2[i];
            //cout << "g " << gnfa1 << endl;
            //cout << "g " << gnfa2 << endl;
            if (gnfa1 != gnfa2) {
                if (gnfa1 -> getRealSymbol() != gnfa2 -> getRealSymbol()) {
            //cout << "Equal out1" << endl;
                    return false;
                }
                NFA<Symbol>* nfa1 = gnfa1 -> getNFA();
                NFA<Symbol>* nfa2 = gnfa2 -> getNFA();
                //cout << "nfa1 " << nfa1 << endl;
                //cout << "nfa2 " << nfa2 << endl;
                DFA<Symbol>* dfa1 = dfaMap[nfa1];
                DFA<Symbol>* dfa2 = dfaMap[nfa2];
                if (!dfa1) {
                    dfa1 = & (nfa1 -> minimize());
                    dfaMap[nfa1] = dfa1;
                }
                if (!dfa2) {
                    dfa2 = & (nfa2 -> minimize());
                    dfaMap[nfa2] = dfa2;
                }
                //cout << "dfa1 " << dfa1 << endl;
                //cout << "dfa2 " << dfa2 << endl;
                ID code1, code2;
                if (codeMap.count(dfa1) > 0) {
                    code1 = codeMap[dfa1];
                } else {
                    code1 = getCode(dfa1);
                    codeMap[dfa1] = code1;
                }
                if (codeMap.count(dfa2) > 0) {
                    code2 = codeMap[dfa2];
                } else {
                    code2 = getCode(dfa2);
                    codeMap[dfa2] = code2;
                }
                //cout << "code1 " << code1 << endl;
                //cout << "code2 " << code2 << endl;
                if (code1 != code2) {
            //cout << "Equal out2" << endl;
                    return false;
                }
                DFA<Symbol>* comDFA1 = comMap[dfa1];
                //cout << "size " << dfa1 -> getAlphabet().size() << endl;
                //cout << "isMini " << dfa1 -> isMinimal() << endl;
                //dfa1 -> output();
                if (!comDFA1) {
                    comDFA1 = &(!(*dfa1));
                    //cout << "finish1" << endl;
                    comMap[dfa1] = comDFA1;
                }
                //cout << "com1 " << comDFA1 << endl;
                if (!((*dfa2 & *comDFA1).isEmpty())) {
            //cout << "Equal out3" << endl;
                    return false;
                }
                DFA<Symbol>* comDFA2 = comMap[dfa2];
                if (!comDFA2) {
                    comDFA2 = &(!(*dfa2));
                    //cout << "finish2" << endl;
                    comMap[dfa2] = comDFA2;
                }
                //cout << "com2 " << comDFA2 << endl;
                if (!((*dfa1 & *(comDFA2)).isEmpty())) {
            //cout << "Equal out4" << endl;
                    return false;
                }
            }
        }
        return true;
    }

    bool DNSS::isNewRelation(NFARelation& relation, NFARelations& relations) {
        //cout << "in" << endl;
        for (ID i = 0; i < relations.size(); i++) {
            if (isEqual(relation, relations[i])) {
                cout << "is Equal" << endl;
                //cout << "out0" << endl;
                return false;
            }
        }
        //cout << "out1" << endl;
        return true;
    }

    bool hasSameSymbol(const Symbols& symbols1, const Symbols& symbols2) {
        for (Symbol symbol : symbols1) {
            if (symbols2.count(symbol) > 0) return true;
        }
        return false;
    }


    void DNSS::addRelationNew(NFARelation& relation, AddressMap& relationAddressMap) {
        GNFA* topGNFA = relation[0];
        for (auto& mapPair : jumpOP2SymbolsMap) {
            JumpOperation* op = mapPair.first;
            Symbol targetSymbol = op -> getTargetSymbol();
            ID add = addressMap[targetSymbol];
            if (addressMap[topGNFA -> getRealSymbol()] == add) continue;
            Symbols& jumpSymbols = mapPair.second;
            if (!hasSameSymbol(topGNFA -> getTopSymbols(), jumpSymbols)) continue;
            GNFA* newTopGNFA = mkTopGNFA(topGNFA, mapPair.second);
            gnfas.insert(newTopGNFA);
            NFARelation newRelation, newerRelation;
            AddressMap newRelationAddressMap, newerRelationAddressMap;
            updateRelation(relation, newTopGNFA, 0, newRelation, newRelationAddressMap);
            if (newRelationAddressMap.count(add) > 0) {
                ID pos = newRelationAddressMap[add];
                GNFA* gnfa = newRelation[pos];
                GNFA* newGNFA = op -> mkGNFA(ctpSymbols, symbolMap, recordSymbolMap, gnfa, pdsMap[add], pdsStateMap);
                gnfas.insert(newGNFA);
                updateRelation(newRelation, newGNFA, pos, newerRelation, newerRelationAddressMap);
            } else {
                GNFA* newGNFA = GNFA::mkGNFA(targetSymbol, targetSymbol, recordAlphabet, ctpSymbols, symbolMap, pdsStateMap);
                newGNFA -> postStar(pdsMap[add], recordSymbolMap);
                gnfas.insert(newGNFA);
                updateRelation(newRelation, newGNFA, newerRelation, newerRelationAddressMap);
            }
            if (isNewRelation(newerRelation, nfaRelations)) {
                cout << "new relation" << endl;
                nfaRelations.push_back(newerRelation);
                newWorkRelations.push_back(newerRelation);
                relationAddressMaps.push_back(newerRelationAddressMap);
                newWorkRelationAddressMaps.push_back(newerRelationAddressMap);
            }
        }
    }

    void DNSS::addRelation(NFARelation& relation, AddressMap& relationAddressMap) {
        GNFA* topGNFA = relation[0];
        for (Symbol topSymbol : topGNFA -> getTopSymbols()) {
            if (jumpTransMap.count(topSymbol) == 0) continue;
            for (JumpOperation* op : jumpTransMap[topSymbol]) {
                GNFA* newTopGNFA = mkTopGNFA(topGNFA, topSymbol);
                gnfas.insert(newTopGNFA);
                NFARelation newRelation, newerRelation;
                AddressMap newRelationAddressMap, newerRelationAddressMap;
                updateRelation(relation, newTopGNFA, 0, newRelation, newRelationAddressMap);
                Symbol targetSymbol = op -> getTargetSymbol();
                ID add = addressMap[targetSymbol];
                if (newRelationAddressMap.count(add) > 0) {
                    ID pos = newRelationAddressMap[add];
                    GNFA* gnfa = newRelation[pos];
                    GNFA* newGNFA = op -> mkGNFA(ctpSymbols, symbolMap, recordSymbolMap, gnfa, pds, pdsStateMap);
                    gnfas.insert(newGNFA);
                    updateRelation(newRelation, newGNFA, pos, newerRelation, newerRelationAddressMap);
                } else {
                    GNFA* newGNFA = GNFA::mkGNFA(targetSymbol, targetSymbol, recordAlphabet, ctpSymbols, symbolMap, pdsStateMap);
                    newGNFA -> postStar(pds, recordSymbolMap);
                    gnfas.insert(newGNFA);
                    updateRelation(newRelation, newGNFA, newerRelation, newerRelationAddressMap);
                }
                if (isNewRelation(newerRelation, nfaRelations)) {
                    cout << "new relation" << endl;
                    nfaRelations.push_back(newerRelation);
                    newWorkRelations.push_back(newerRelation);
                    relationAddressMaps.push_back(newerRelationAddressMap);
                    newWorkRelationAddressMaps.push_back(newerRelationAddressMap);
                }
            }
        }

    }

    void DNSS::updateRelation(NFARelation& relation, GNFA* gnfa, ID pos, NFARelation& newRelation, AddressMap& newRelationAddressMap) {
        newRelation.push_back(gnfa);
        ID add = addressMap[gnfa -> getRealSymbol()];
        newRelationAddressMap[add] = 0;
        for (ID i = 0; i < relation.size(); i++) {
            if (i != pos) {
                GNFA* gnfa = relation[i];
                newRelation.push_back(gnfa);
                ID add = addressMap[gnfa -> getRealSymbol()];
                if (i < pos) {
                    newRelationAddressMap[add] = i + 1;
                } else {
                    newRelationAddressMap[add] = i;
                }
            }
        }
    }

    void DNSS::updateRelation(NFARelation& relation, GNFA* gnfa, NFARelation& newRelation, AddressMap& newRelationAddressMap) {
        newRelation.push_back(gnfa);
        ID add = addressMap[gnfa -> getRealSymbol()];
        newRelationAddressMap[add] = 0;
        for (ID i = 0; i < relation.size(); i++) {
            GNFA* gnfa = relation[i];
            newRelation.push_back(gnfa);
            ID add = addressMap[gnfa -> getRealSymbol()];
            newRelationAddressMap[add] = i + 1;
        }
    }
}
