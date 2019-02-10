/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C,
 * it is licensed under the BSD 3-Clause license; you may not use this file
 * except in compliance with the License.
 *
 * You may obtain a copy of the License at:
 *  https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IOTC_COROUTINE_H__
#define __IOTC_COROUTINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* This block of code below must not be clang-formatted */
// clang-format off

#define IOTC_CR_START( state )\
    switch( state )\
    { \
        default:

#define IOTC_CR_YIELD( state, ret )\
    state = __LINE__; return ret; case __LINE__:

#define IOTC_CR_YIELD_ON( state, expression, ret )\
    if ( (expression) ) \
    { \
      state = __LINE__; return ret; case __LINE__:; \
    }

#define IOTC_CR_YIELD_UNTIL( state, expression, ret )\
    if ( (expression) ) \
    { \
      state = __LINE__; return ret; case __LINE__:; \
      continue; \
    }

#define IOTC_CR_EXIT( state, ret )\
    state = 1; return ret;

#define IOTC_CR_RESTART( state, ret )\
    state = 0; return ret;

#define IOTC_CR_END()\
    };

// clang-format on

#define IOTC_CR_IS_RUNNING(state) (state > 2)

#define IOTC_CR_RESET(state) state = 2

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_COROUTINE_H__ */
