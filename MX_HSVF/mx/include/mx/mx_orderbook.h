#ifndef _MX_ORDERBOOK_H_
#define _MX_ORDERBOOK_H_

#include "mx_log.h"

namespace ns {

struct Level {
    Level(const int64_t price, const int32_t qty) : price(price), qty(qty) 
    {
    }
    int64_t price;
    int32_t qty;
};

class MXOrderbook {
public:
    MXOrderbook()
    {
    }

    void OnNewOrChange(const MarketBookSide side, const size_t level, int64_t price, int32_t qty) {
        if(side == MarketBookSide::Bid) {       _OnNewOrChange(_bids, level, price, qty); }
        else if(side == MarketBookSide::Ask) {  _OnNewOrChange(_asks, level, price, qty); }
        else {
            assert(!"OnNewOrChange - unhandled side");
        }
    }

    void OnDeleteFrom(const MarketBookSide side, const size_t level) {
        if(side == MarketBookSide::Bid) {       _DeleteFrom(_bids, level); }
        else if (side == MarketBookSide::Ask) { _DeleteFrom(_asks, level); }
        else {
            assert(!"OnDeleteFrom - unhandled side");
        }
    }

    std::tuple<bool, int64_t, int32_t> TopBidEqualsTopAsk() const {
        const bool isEqual = _bids.size() > 0 && _asks.size() > 0 && _bids[0].price == _asks[0].price;
        if(isEqual) {
            const int32_t minQty = std::min(_bids[0].qty, _asks[0].qty);
            return std::make_tuple(isEqual, _bids[0].price, minQty);
        }
        return std::make_tuple(isEqual, 0, 0);
    }  

private:
    void _OnNewOrChange(std::vector<Level>& orders, const size_t level, int64_t price, int32_t qty) {
        if(level == orders.size()) {
            orders.push_back({price, qty});
        } else if(level < orders.size()) {
            orders[level].price = price;
            orders[level].qty = qty;
        } else {
            assert(!"_OnNewOrChange - unhandled clause");
        }
    }

    void _DeleteFrom(std::vector<Level>& orders, const size_t level) {
        if(level + 1 > orders.size()) 
            return;

        orders.erase(std::begin(orders) + level, std::end(orders));
    }

    std::vector<Level> _bids, _asks;
};



class MXOrderbooks {
public:
    void NewOrChange(const std::string& identifier, const MarketBookSide side, const size_t level, int64_t price, int32_t qty) {
        MXOrderbook& orderbook = CreateOrGetOrderbook(identifier);
        orderbook.OnNewOrChange(side, level, price, qty);
    }
    void DeleteFrom(const std::string& identifier, const MarketBookSide side, const size_t level) {
        MXOrderbook& orderbook = CreateOrGetOrderbook(identifier);
        orderbook.OnDeleteFrom(side, level);
    }

    std::tuple<bool, int64_t, int32_t> TopBidEqualsTopAsk(const std::string& identifier) {
        auto it = _orderbooks.find(identifier);
        if(it != std::end(_orderbooks)) {
            return it->second.TopBidEqualsTopAsk();
        }
        return std::make_tuple(false, 0, 0);
    }

    MXOrderbook& CreateOrGetOrderbook(const std::string& identifier) {
        auto pair = _orderbooks.try_emplace(identifier, MXOrderbook());
        return pair.first->second;
    }

private:
    std::unordered_map<std::string, MXOrderbook> _orderbooks;
};


}//end namespace

#endif
