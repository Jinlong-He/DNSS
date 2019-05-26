//
//  Parse.hpp
//  ASM
//
//  Created by 何锦龙 on 2018/8/22.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef Parse_hpp
#define Parse_hpp

#include <stdio.h>
#include <string>
#include <ostream>
#include <fstream>
#include "Activity.hpp"
#include "../../Utility/Utility.hpp"
using namespace std;
namespace atm {
    enum error {lmError, unmatchError, noMainError};
    /// \brief This class is to get information via reading files.
    class Parse
    {
        Aft mainAft;                                    ///< the main affinity.
        Activity* mainActivity;                         ///< the main activity.
        Afts afts;                                      ///< all affinities from reading info file.
        Acts activities;                                ///< all activities from reading info file.
        Actions actions;                                 ///< all actions from reading trans file.
        unordered_map<string, Activity*> str2ActMap;    ///< a map for matching string and Activity.
        string infoFileName;
    public:
        Parse() {}
        Parse(const string& infoFileName, const string& transFileName);
        ~Parse() {
            for (Activity* act : activities)
                delete act;
            for (Action* action : actions)
                delete action;
        }
        Acts& getActivities() {return activities;}
        Activity* getMainActivity() {return mainActivity;}
        Actions& getActions() {return actions;}
        Afts& getAfts() {return afts;}
        Aft getMainAft() {return mainAft;}

        void readInfoFile(const string& fileName);
        void readTransitionGragh(const string& fileName);
        void writeErr(int error);
    };
}


#endif /* Parse_hpp */
