//
//  GNFA.hpp
//
//  Created by 何锦龙 on 2019/5/17.
//  Copyright © 2019年 何锦龙. All rights reserved.
//

#ifndef GNFA_hpp
#define GNFA_hpp
#include "stdio.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../../CGH/include/CGH.hpp"
using namespace std;
using namespace cgh;
namespace dnss {
    enum PDSStateType {eps, bot, p};
    typedef size_t Symbol;
    typedef size_t ID;
    typedef pair<Symbol, Symbol> Symbol2;
    typedef unordered_set<Symbol> Symbols;
    typedef pair<Symbol, Symbols> RecordSymbol;
    typedef unordered_map<RecordSymbol, Symbol> SymbolMap;
    typedef unordered_map<size_t, PDSState*> PDSStateMap;
    typedef vector<RecordSymbol> RecordSymbolVec;
    typedef vector<Symbols> SymbolsVec;
    typedef cgh::Alias4FA<Symbol>::NFAState2Map NFAState2Map;
    typedef cgh::Alias4FA<Symbol>::DFAState2NFAStateMap DFAState2NFAStateMap;
    typedef cgh::Alias4FA<Symbol>::NFAStates NFAStates;
    typedef cgh::Alias4FA<Symbol>::DFAStates DFAStates;
    typedef cgh::Alias4FA<Symbol>::NFATransMap NFATransMap;
    typedef cgh::Alias4FA<Symbol>::StateChar StateChar;
    typedef cgh::Alias4FA<Symbol>::PostStarMap PostStarMap;
    typedef cgh::Alias4FA<Symbol>::FAList FAList;
    typedef cgh::Alias4FA<Symbol>::FAs FAs;
    typedef cgh::Alias4PDS<Symbol>::PDSState2NFAStateMap PDSState2NFAStateMap;
    
    class GNFA {
    private:
        NFA<Symbol>* nfa;
        PDSState2NFAStateMap state2Map;
        PostStarMap postStarMap;
        Symbol realSymbol;
        Symbols topSymbols;
    public:
        GNFA() : nfa(nullptr) {}

        GNFA(NFA<Symbol>* a, PDSState2NFAStateMap& sMap, PostStarMap& pMap, Symbol s) : nfa(a), state2Map(sMap), postStarMap(pMap), realSymbol(s) {}

        ~GNFA() {
            delete nfa;
        }

        NFA<Symbol>* getNFA() {
            return nfa;
        }

        NFA<Symbol>* getNFA() const {
            return nfa;
        }

        PDSState2NFAStateMap& getState2Map() {
            return state2Map;
        }

        PostStarMap& getPostStarMap() {
            return postStarMap;
        }

        Symbol getRealSymbol() {
            return realSymbol;
        }

        Symbol getRealSymbol() const {
            return realSymbol;
        }

        Symbols& getTopSymbols() {
            return topSymbols;
        }

        void addTopSymbol(Symbol s) {
            topSymbols.insert(s);
        }

        void postStar(PDS<Symbol>* pds, RecordSymbolVec& recordSymbolMap) {
            nfa -> postStar(*pds, state2Map, postStarMap);
            for (NFAState<Symbol>* state : nfa -> getStates()) {
                NFAStates epsilonClosure = state -> getEpsilonClosure();
                for (NFAState<Symbol>* epsState : epsilonClosure) {
                    for (auto& mapPair : epsState -> getTransMap()) {
                        Symbol symbol = mapPair.first;
                        if (symbol == 0) continue;
                        state -> getTransMap()[symbol].insert(mapPair.second.begin(), mapPair.second.end());
                    }
                }
            }
            for (NFAState<Symbol>* state : nfa -> getStates()) {
                state -> delNFATrans((Symbol)0);
            }
            for (auto& mapPair : nfa -> getInitialState() -> getTransMap()) {
                Symbol symbol = mapPair.first;
                topSymbols.insert(recordSymbolMap[symbol].first);
            }
        }

        bool operator == (const GNFA& gnfa) const {
            if (*nfa == *(gnfa.getNFA()) && realSymbol == gnfa.getRealSymbol()) {
                return true;
            }
            return false;
        }

        static GNFA* mkGNFA(Symbol symbol, Symbol rSymbol, Symbols& recordAlphabet, Symbols& ctpSymbols, SymbolMap& symbolMap, PDSStateMap& pdsStateMap) {
            PDSState2NFAStateMap state2Map;
            PostStarMap postStarMap;
            NFA<Symbol>* nfa = new NFA<Symbol>(recordAlphabet);
            NFAState<Symbol>* initialState = nfa -> mkInitialState();
            NFAState<Symbol>* botState = nfa -> mkState();
            NFAState<Symbol>* epsState = nfa -> mkState();
            NFAState<Symbol>* fState = nfa -> mkFinalState();
            state2Map[pdsStateMap[p]] = initialState;
            state2Map[pdsStateMap[bot]] = botState;
            state2Map[pdsStateMap[eps]] = epsState;
            for (Symbol s : recordAlphabet) {
                postStarMap[StateChar(initialState, s)] = nfa -> mkState();
                postStarMap[StateChar(botState, s)] = nfa -> mkState();
                postStarMap[StateChar(epsState, s)] = nfa -> mkState();
            }
            Symbols symbols;
            Symbol tsymbol = 0;
            if (ctpSymbols.count(symbol) == 0) {
                tsymbol = symbolMap[RecordSymbol(symbol, symbols)];
            } else {
                symbols.insert(symbol);
                tsymbol = symbolMap[RecordSymbol(symbol, symbols)];
            }
            initialState -> addTrans(tsymbol, botState);
            botState -> addTrans(bot, fState);
            GNFA* gnfa = new GNFA(nfa, state2Map, postStarMap, rSymbol);
            return gnfa;
        }

        static GNFA* cpGNFA(GNFA* gnfa, NFAState2Map& copyMap) {
            NFA<Symbol>* nfa = gnfa -> getNFA();
            PDSState2NFAStateMap& state2Map = gnfa -> getState2Map();
            PostStarMap& postStarMap = gnfa -> getPostStarMap();
            Symbol realSymbol = gnfa -> getRealSymbol();
            PDSState2NFAStateMap newStateMap;
            PostStarMap newPostStarMap;
            NFA<Symbol>* newNFA = new NFA<Symbol>(nfa -> getAlphabet());
            for (NFAState<Symbol>* state : nfa -> getStates()) {
                NFAState<Symbol>* newState = nullptr;
                if (state == nfa -> getInitialState()) {
                    newState = newNFA -> mkInitialState();
                } else if (state -> isFinal()) {
                    newState = newNFA -> mkFinalState();
                } else {
                    newState = newNFA -> mkState();
                }
                copyMap[state] = newState;
            }
            for (auto& mapPair : state2Map) {
                newStateMap[mapPair.first] = copyMap[mapPair.second];
            }
            for (auto& mapPair : postStarMap) {
                NFAState<Symbol>* state = mapPair.first.first;
                Symbol symbol = mapPair.first.second;
                newPostStarMap[StateChar(copyMap[state], symbol)] = copyMap[mapPair.second];
            }
            GNFA* newGNFA = new GNFA(newNFA, newStateMap, newPostStarMap, realSymbol);
            return newGNFA;
        }

        static void cpNFA(NFAState2Map& copyMap, NFAStates& work) {
            NFAStates visited(work), newWork;
            while (work.size() > 0) {
                for (NFAState<Symbol>* state : work) {
                    NFAState<Symbol>* newState = copyMap[state];
                    for (auto& mapPair : state -> getTransMap()) {
                        Symbol symbol = mapPair.first;
                        for (NFAState<Symbol>* targetState : mapPair.second) {
                            newState -> addTrans(symbol, copyMap[targetState]);
                            if (visited.insert(targetState).second) newWork.insert(targetState);
                        }
                    }
                }
                work.clear();
                if (newWork.size() > 0) {
                    work.insert(newWork.begin(), newWork.end());
                    newWork.clear();
                }
            }
        }

    };

}

    

#endif /* GNFA_hpp */
