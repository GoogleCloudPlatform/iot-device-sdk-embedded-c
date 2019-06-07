/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
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

#include "iotc_itest_helpers.h"

int cmocka_run_test_groups(const struct CMGroupTest* const tgroups) {
  int ret = 0;

  int i = 0;
  while (tgroups[i].tests) {
    ret = _cmocka_run_group_tests(tgroups[i].name, tgroups[i].tests,
                                  tgroups[i].len, NULL, NULL);

    if (ret != 0) {
      return ret;
    }

    ++i;
  }

  return ret;
}

void iotc_itest_ptr_cmp(iotc_mock_cmp_t* cmp) {
  switch (cmp->cmp_type) {
    case CMP_TYPE_EQUAL:
      assert_ptr_equal(cmp->lvalue, cmp->rvalue);
      break;
    case CMP_TYPE_NOT_EQUAL:
      assert_ptr_not_equal(cmp->lvalue, cmp->rvalue);
      break;
  }
}

iotc_layer_t* iotc_itest_find_layer(iotc_context_t* iotc_context,
                                    int layer_id) {
  iotc_layer_t* root = iotc_context->layer_chain.top;

  while (root) {
    if (root->layer_type_id == layer_id) {
      return root;
    }

    root = root->layer_connection.prev;
  }

  return root;
}

#define iotc_itest_inject_wrap(layer, wrap_name, wrap) \
  if (wrap) {                                          \
    layer->layer_funcs->wrap_name = wrap;              \
  }

void iotc_itest_inject_wraps(iotc_context_t* iotc_context, int layer_id,
                             iotc_layer_func_t* push, iotc_layer_func_t* pull,
                             iotc_layer_func_t* close,
                             iotc_layer_func_t* close_externally,
                             iotc_layer_func_t* init,
                             iotc_layer_func_t* connect) {
  // first find proper layer
  iotc_layer_t* layer = iotc_itest_find_layer(iotc_context, layer_id);

  if (layer) {
    iotc_itest_inject_wrap(layer, push, push);
    iotc_itest_inject_wrap(layer, pull, pull);
    iotc_itest_inject_wrap(layer, close, close);
    iotc_itest_inject_wrap(layer, close_externally, close_externally);
    iotc_itest_inject_wrap(layer, init, init);
    iotc_itest_inject_wrap(layer, connect, connect);
  } else {
    fail_msg("Could not find layer type: %d\n", layer_id);
  }
}
