/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include "util/u_math.h"
#include "util/u_vector.h"

int
u_vector_init(struct u_vector *vector, uint32_t element_size, uint32_t size)
{
   assert(util_is_power_of_two(size));
   assert(element_size < size && util_is_power_of_two(element_size));

   vector->head = 0;
   vector->tail = 0;
   vector->element_size = element_size;
   vector->size = size;
   vector->data = malloc(size);

   return vector->data != NULL;
}

void *
u_vector_add(struct u_vector *vector)
{
   uint32_t offset, size, split, src_tail, dst_tail;
   void *data;

   if (vector->head - vector->tail == vector->size) {
      size = vector->size * 2;
      data = malloc(size);
      if (data == NULL)
         return NULL;
      src_tail = vector->tail & (vector->size - 1);
      dst_tail = vector->tail & (size - 1);
      if (src_tail == 0) {
         /* Since we know that the vector is full, this means that it's
          * linear from start to end so we can do one copy.
          */
         memcpy((char *)data + dst_tail, vector->data, vector->size);
      } else {
         /* In this case, the vector is split into two pieces and we have
          * to do two copies.  We have to be careful to make sure each
          * piece goes to the right locations.  Thanks to the change in
          * size, it may or may not still wrap around.
          */
         split = u_align_u32(vector->tail, vector->size);
         assert(vector->tail <= split && split < vector->head);
         memcpy((char *)data + dst_tail, (char *)vector->data + src_tail,
                split - vector->tail);
         memcpy((char *)data + (split & (size - 1)), vector->data,
                vector->head - split);
      }
      free(vector->data);
      vector->data = data;
      vector->size = size;
   }

   assert(vector->head - vector->tail < vector->size);

   offset = vector->head & (vector->size - 1);
   vector->head += vector->element_size;

   return (char *)vector->data + offset;
}

void *
u_vector_remove(struct u_vector *vector)
{
   uint32_t offset;

   if (vector->head == vector->tail)
      return NULL;

   assert(vector->head - vector->tail <= vector->size);

   offset = vector->tail & (vector->size - 1);
   vector->tail += vector->element_size;

   return (char *)vector->data + offset;
}
