#include "Operation.hpp"
//cgh::ID State::counter = 0;
//cgh::ID Var::counter = 0;
//cgh::ID Value::counter = 0;
//unordered_set<Object*> Manage::buffer;
//Manage manage;
namespace dnss {
    void POP::mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& state2Map) {
        PDSState* pState = state2Map[p];
        for (Symbols symbols : powerset) {
            Symbol symbol = symbolMap[RecordSymbol(sourceSymbol, symbols)];
            pds -> mkPopPDSTrans(pState, pState, symbol);
        }
    }

    void CTK::mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& state2Map) {
            Symbol symbol = 0;
            Symbols symbols;
            if (ctpSymbols.count(targetSymbol) == 0) {
                symbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
            } else {
                symbols.insert(targetSymbol);
                symbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
            }
            PDSState* botState = state2Map[bot];
            PDSState* pState = state2Map[p];
            pds -> mkPushPDSTrans(botState, pState, bot, Symbol2(symbol, bot));
    }

    void PUSH::mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& state2Map) {
        PDSState* pState = state2Map[p];
        for (Symbols symbols : powerset) {
            Symbol ssymbol = symbolMap[RecordSymbol(sourceSymbol, symbols)];
            Symbol tsymbol = 0;
            if (ctpSymbols.count(targetSymbol) == 0) {
                tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
            } else {
                symbols.insert(targetSymbol);
                tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
            }
            pds -> mkPushPDSTrans(pState, pState, ssymbol, Symbol2(tsymbol, ssymbol));
        }
    }

    void CTP::mkPDSTransition(Symbol sourceSymbol, Symbols& alphabet, Symbols& ctpSymbols, SymbolsVec& powerset, SymbolMap& symbolMap, PDS<Symbol>* pds, PDSStateMap& state2Map) {
        PDSState* pState = state2Map[p];
        PDSState* epsState = state2Map[eps];
        for (Symbols symbols : powerset) {
            Symbol ssymbol = symbolMap[RecordSymbol(sourceSymbol, symbols)];
            if (symbols.count(targetSymbol) == 0) {
                symbols.insert(targetSymbol);
                Symbol tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
                pds -> mkPushPDSTrans(pState, pState, ssymbol, Symbol2(tsymbol, ssymbol));
            } else {
                pds -> mkPopPDSTrans(pState, epsState, ssymbol);
                Symbol tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
                pds -> mkReplacePDSTrans(epsState, pState, tsymbol, tsymbol);
                for (Symbol s : alphabet) {
                    Symbol symbol = symbolMap[RecordSymbol(s, symbols)];
                    pds -> mkPopPDSTrans(epsState, epsState, symbol);
                }
            }
        }
    }


    GNFA* JumpPUSH::mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap) {
        NFAState2Map copyMap;
        GNFA* newGNFA = GNFA::cpGNFA(gnfa, copyMap);
        PostStarMap& postStarMap = gnfa -> getPostStarMap();
        NFA<Symbol>* nfa = gnfa -> getNFA();
        NFA<Symbol>* newNFA = newGNFA -> getNFA();
        NFAState<Symbol>* initialState = nfa -> getInitialState();
        NFAState<Symbol>* newInitialState = newNFA-> getInitialState();
        NFAStates work;
        for (auto& mapPair : initialState -> getTransMap()) {
            RecordSymbol recordSymbol = recordSymbolMap[mapPair.first];
            Symbol symbol = recordSymbol.first;
            Symbols symbols = recordSymbol.second;
            Symbol tsymbol = 0;
            if (ctpSymbols.count(targetSymbol) == 0) {
                tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
            } else {
                symbols.insert(targetSymbol);
                tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
            }
            NFAState<Symbol>* midState = postStarMap[StateChar(initialState, tsymbol)];
            NFAState<Symbol>* newMidState = copyMap[midState];
            newInitialState -> addTrans(tsymbol, newMidState);
            for (NFAState<Symbol>* targetState : mapPair.second) {
                newMidState -> addTrans(mapPair.first, copyMap[targetState]);
                work.insert(targetState);
            }
        }
        GNFA::cpNFA(copyMap, work);
        newGNFA -> postStar(pds, recordSymbolMap);
        return newGNFA;
    }

    GNFA* JumpCTP::mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap) {
        NFAState2Map copyMap;
        GNFA* newGNFA = GNFA::cpGNFA(gnfa, copyMap);
        PostStarMap& postStarMap = gnfa -> getPostStarMap();
        NFA<Symbol>* nfa = gnfa -> getNFA();
        NFA<Symbol>* newNFA = newGNFA -> getNFA();
        NFAState<Symbol>* initialState = nfa -> getInitialState();
        NFAState<Symbol>* newInitialState = newNFA-> getInitialState();
        NFAStates work;
        for (auto& mapPair : initialState -> getTransMap()) {
            RecordSymbol recordSymbol = recordSymbolMap[mapPair.first];
            Symbol symbol = recordSymbol.first;
            Symbols symbols = recordSymbol.second;
            if (symbols.count(targetSymbol) == 0) {
                symbols.insert(targetSymbol);
                Symbol tsymbol = symbolMap[RecordSymbol(targetSymbol, symbols)];
                NFAState<Symbol>* midState = postStarMap[StateChar(initialState, tsymbol)];
                cout << midState << endl;
                NFAState<Symbol>* newMidState = copyMap[midState];
                newInitialState -> addTrans(tsymbol, newMidState);
                for (NFAState<Symbol>* targetState : mapPair.second) {
                    newMidState -> addTrans(mapPair.first, copyMap[targetState]);
                    work.insert(targetState);
                }
            }
            if (symbol == targetSymbol) {
                for (NFAState<Symbol>* targetState : mapPair.second) {
                    newInitialState -> addTrans(mapPair.first, copyMap[targetState]);
                    work.insert(targetState);
                }
            }
        }
        GNFA::cpNFA(copyMap, work);
        newGNFA -> postStar(pds, recordSymbolMap);
        return newGNFA;
    }

    GNFA* JumpCTK::mkGNFA(Symbols& ctpSymbols, SymbolMap& symbolMap, RecordSymbolVec& recordSymbolMap, GNFA* gnfa, PDS<Symbol>* pds, PDSStateMap& pdsStateMap) {
        GNFA* newGNFA = GNFA::mkGNFA(targetSymbol, gnfa -> getRealSymbol(), gnfa -> getNFA() -> getAlphabet(), ctpSymbols, symbolMap, pdsStateMap);
        newGNFA -> postStar(pds, recordSymbolMap);
        return newGNFA;
    }

}

