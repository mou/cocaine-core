//
// Copyright (C) 2011-2012 Andrey Sibiryov <me@kobology.ru>
//
// Licensed under the BSD 2-Clause License (the "License");
// you may not use this file except in compliance with the License.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef COCAINE_RPC_HPP
#define COCAINE_RPC_HPP

#include <boost/mpl/map.hpp>

#include "cocaine/events.hpp"
#include "cocaine/job.hpp"

namespace cocaine { namespace engine { namespace rpc {    
    enum codes {
        heartbeat,
        terminate,
        invoke,
        push,
        error,
        release
    };

    // Generic packer
    // --------------

    using namespace boost::mpl;

    typedef map<
        pair< events::heartbeat_t, int_<heartbeat> >,
        pair< events::terminate_t, int_<terminate> >,
        pair< events::release_t,   int_<release  > >
    > codemap;

    template<typename T> 
    struct packed {
        typedef boost::tuple<int> type;

        packed():
            pack(typename at<codemap, T>::type())
        { }

        const type& get() const {
            return pack;
        }

        type pack;
    };

    // Specific packers
    // ----------------

    template<>
    struct packed<events::invoke_t> {
        typedef boost::tuple<int, const std::string&, zmq::message_t&> type;

        // XXX: Test whether this zero-copy magic never backfires.
        packed(const events::invoke_t& event):
            message(event.job->request().data(), 
                    event.job->request().size(), 
                    NULL),
            pack(invoke, event.job->method(), message)
        { }

        const type& get() const {
            return pack;
        }

        zmq::message_t message;
        type pack;
    };

    template<>
    struct packed<events::push_t> {
        typedef boost::tuple<int, zmq::message_t&> type;

        packed(const events::push_t& event):
            pack(push, event.message)
        { }

        const type& get() const {
            return pack;
        }

        type pack;
    };

    template<>
    struct packed<events::error_t> {
        typedef boost::tuple<int, int, const std::string&> type;

        packed(const events::error_t& event):
            pack(error, event.code, event.message)
        { }

        const type& get() const {
            return pack;
        }

        type pack;
    };

    template<typename T>
    const typename packed<T>::type& pack(const T& event) {
        return packed<T>(event).get();
    }
}}}

#endif
