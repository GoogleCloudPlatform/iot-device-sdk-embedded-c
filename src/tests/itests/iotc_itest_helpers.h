/* Copyright 2018 - 2019 Google LLC
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

#ifndef IOTC_ITEST_HELPERS_H
#define IOTC_ITEST_HELPERS_H

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>  // IAR EWARM limitation: comocka.h must come after all
#include <iotc.h>
#include <iotc_event_handle.h>
#include <iotc_layers_ids.h>
#include <iotc_macros.h>
#include <iotc_mqtt_logic_layer.h>
#include <iotc_mqtt_logic_layer_data.h>
#include <iotc_types_internal.h>
// headers that may reference stdlib.h.

struct CMGroupTest {
  struct CMUnitTest* tests;
  const char* name;
  int len;
  CMFixtureFunction group_setup;
  CMFixtureFunction group_teardown;
};

#define cmocka_test_group(group) \
  { group, #group, IOTC_ARRAYSIZE(group), NULL, NULL }

#define cmocka_test_group_end \
  { NULL, NULL, 0, NULL, NULL }

/**
 * @brief The CMP_TYPE enum
 * Type of the comparation
 */
enum CMP_TYPE { CMP_TYPE_EQUAL, CMP_TYPE_NOT_EQUAL };

enum PROC_TYPE { PROC_TYPE_DONT, PROC_TYPE_DO };

/**
 * @brief The iotc_mock_cmp struct
 *
 * Helper struct for passing the conditions to check, this
 * way it's possible to reuse the same wrapper in a multiple situations.
 *
 */
typedef struct iotc_mock_cmp_s {
  void* lvalue;
  enum CMP_TYPE cmp_type;
  void* rvalue;
} iotc_mock_cmp_t;

/**
 * type that represents function that will be passed to test various of states
 * of layers and contexts
 * will make the code to be reusable and easily extendable
 */
typedef void(test_state_fun_t)(void* context, void* data,
                               iotc_state_t in_out_state);

/**
 * macro for safe evaluation of test state function
 */
#define test_state_evaluate(fun, context, data, in_out_state) \
  if ((fun)) {                                                \
    (*(fun))(context, data, in_out_state);                    \
  }

/**
 * @brief cmocka_run_test_groups
 *
 * helper function that enables running series of test groups by cmocka
 */
extern int cmocka_run_test_groups(const struct CMGroupTest* const tgroups);

/**
 * @brief iotc_itest_ptr_cmp
 * @param cmp
 *
 * Checks if the passed condition is fulfield
 */
extern void iotc_itest_ptr_cmp(iotc_mock_cmp_t* cmp);

/**
 * @brief iotc_itest_find_layer
 *
 * helper function that allows to find a pointer to a layer on the layer's
 * stack so that it can be used to exchange pointers etc.
 *
 * @param iotc_context
 * @param layer_id
 * @return
 */
extern iotc_layer_t* iotc_itest_find_layer(iotc_context_t* iotc_context,
                                           int layer_id);

/**
 * @brief iotc_itest_inject_wraps
 *
 * helper that allows to inject wrappers into the layer system so it's possible
 * to inject all functions or only one parameters may be equall to NULL which
 *means
 * do not change that pointer.
 *
 */
extern void iotc_itest_inject_wraps(iotc_context_t* iotc_context, int layer_id,
                                    iotc_layer_func_t* push,
                                    iotc_layer_func_t* pull,
                                    iotc_layer_func_t* close,
                                    iotc_layer_func_t* close_externally,
                                    iotc_layer_func_t* init,
                                    iotc_layer_func_t* connect);

/**
 * @brief iotc_create_context_with_custom_layers
 *
 * hidden API function that allows us to initialize context with custom
 * layer setup
 *
 * @param context
 * @param layer_config
 * @param layer_chain
 * @param layer_chain_size
 * @return
 */
extern iotc_state_t iotc_create_context_with_custom_layers(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    iotc_layer_type_id_t layer_chain[], size_t layer_chain_size);

/**
 * @brief iotc_delete_context_with_custom_layers
 *
 * hidden API function that allows us to delete context with custom
 * layer setup
 *
 * @param context
 * @param layer_config
 * @return
 */
extern iotc_state_t iotc_delete_context_with_custom_layers(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    size_t layer_chain_size);

/**
 * macros for extracting next, and prev layer user data - this will be replaced
 * by the lookup function
 */
#define IOTC_PREV_LAYER(context) ((iotc_layer_connectivity_t*)context)->prev
#define IOTC_NEXT_LAYER(context) ((iotc_layer_connectivity_t*)context)->next

#endif  // IOTC_ITEST_HELPERS_H
