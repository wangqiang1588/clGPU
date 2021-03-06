// Copyright (c) 2017-2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "functions/Sscal.hpp"

static const char* module_name = "Sscal_block_read";
static const char* kernel_name = "Sscal_block_read";

static const int simd = 16;
static const int vec_size = 4;
static const int wi_elems = 1;

static const int lwg_elems = simd * vec_size * wi_elems;

namespace iclgpu { namespace functions { namespace implementations {

bool Sscal_block_read::accept(const Sscal::params& params, Sscal::score& score)
{
    if (params.incx != 1 || params.n < lwg_elems)
        return false;
    score.incx = 1.2f;
    score.n = 1.6f;
    return true;

}

event Sscal_block_read::execute(const Sscal::params& params, const std::vector<event>& dep_events)
{
    auto engine = context()->get_engine();
    auto kernel = engine->get_kernel(kernel_name, module_name);
    auto buf_size = params.n * params.incx;

    kernel->set_arg(0, params.n);
    kernel->set_arg(1, params.alpha);
    auto buf_x = engine->get_inout_buffer(params.x, buf_size);
    kernel->set_arg(2, buf_x);

    auto lwg_number = params.n / lwg_elems;

    auto gws = nd_range(lwg_number * simd);
    auto lws = nd_range(simd);

    kernel->set_options({ gws, lws });

    return kernel->submit(dep_events);
}

} } } // namespace iclgpu::functions::implementations
