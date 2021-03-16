//
// Created by acco on 22/10/19.
//

#ifndef FILO__BPP_HPP_
#define FILO__BPP_HPP_


#include <algorithm>


namespace bpp {

    int greedy_first_fit_decreasing(const cobra::Instance &instance) {

        auto customers = std::vector<int>();
        customers.reserve(instance.get_customers_num());

        for (auto i = instance.get_customers_begin(); i < instance.get_customers_end(); i++) {
            customers.emplace_back(i);
        }

        std::sort(customers.begin(), customers.end(),
                  [&instance](auto i, auto j) { return instance.get_demand(i) > instance.get_demand(j); });

        struct bin {
            int load{};
            std::vector<int> customers{};
        };

        auto bins = std::vector<bin>(instance.get_customers_num());

        auto used_bins = 0ul;
        for(auto i : customers) {
            const auto i_demand = instance.get_demand(i);
            for(auto p = 0ul; p < bins.size(); p++) {
                auto& bin = bins[p];
                if(bin.load + i_demand <= instance.get_vehicle_capacity()) {
                    bin.load += i_demand;
                    bin.customers.push_back(i);
                    if(p+1 > used_bins) {
                        used_bins = p + 1;
                    }
                    break;
                }
            }
        }

        return used_bins;

    }

}



#endif //FILO__BPP_HPP_
