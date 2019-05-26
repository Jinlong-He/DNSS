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
                addRelation(workRelations[i], workRelationAddressMaps[i]);
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

    ID getCode(DFA<Symbol>* dfa) {
        DFAState<Symbol>* initialState = dfa -> getInitialState();
        ID code = hash<double>()((initialState -> getTransMap().size()) * 3.14);
        for (auto& mapPair : initialState -> getTransMap()) {
            code ^= mapPair.first;
        }
        return code;
    }

    bool DNSS::isEqual(NFARelation& relation1, NFARelation& relation2) {
        if (relation1.size() != relation2.size()) return false;
        for (ID i = 0; i < relation1.size(); i++) {
            GNFA* gnfa1 = relation1[i];
            GNFA* gnfa2 = relation2[i];
            if (gnfa1 != gnfa2) {
                if (gnfa1 -> getRealSymbol() != gnfa2 -> getRealSymbol()) return false;
                NFA<Symbol>* nfa1 = gnfa1 -> getNFA();
                NFA<Symbol>* nfa2 = gnfa2 -> getNFA();
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
                if (code1 != code2) return false;
                DFA<Symbol>* comDFA1 = comMap[dfa1];
                if (!comDFA1) {
                    comDFA1 = &(!(*dfa1));
                    comMap[dfa1] = comDFA1;
                }
                if (!((*dfa2 & *(comDFA1)).isEmpty())) return false;
                DFA<Symbol>* comDFA2 = comMap[dfa2];
                if (!comDFA2) {
                    comDFA2 = &(!(*dfa2));
                    comMap[dfa2] = comDFA2;
                }
                if (!((*dfa1 & *(comDFA2)).isEmpty())) return false;
            }
        }
        return true;
    }

    bool DNSS::isNewRelation(NFARelation& relation, NFARelations& relations) {
        for (ID i = 0; i < relations.size(); i++) {
            if (isEqual(relation, relations[i])) {
                return false;
            }
        }
        return true;
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
