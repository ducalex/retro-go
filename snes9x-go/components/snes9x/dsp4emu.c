/* This file is part of Snes9x. See LICENSE file. */

#include "dsp4.h"
#include "memmap.h"

#define DSP4_READ_WORD(x) \
   READ_WORD(DSP4.parameters+x)

#define DSP4_WRITE_WORD(x,d) \
   WRITE_WORD(DSP4.output+x,d);

/* used to wait for dsp i/o */
#define DSP4_WAIT(x) \
    DSP4_Logic = x; \
    return

int32_t DSP4_Multiply(int16_t Multiplicand, int16_t Multiplier)
{
   return Multiplicand * Multiplier;
}

int16_t DSP4_UnknownOP11(int16_t A, int16_t B, int16_t C, int16_t D)
{
   return ((A * 0x0155 >>  2) & 0xf000) | ((B * 0x0155 >>  6) & 0x0f00) | ((C * 0x0155 >> 10) & 0x00f0) | ((D * 0x0155 >> 14) & 0x000f);
}

void DSP4_Op06(bool size, bool msb)
{
   /* save post-oam table data for future retrieval */
   op06_OAM[op06_index] |= (msb << (op06_offset + 0));
   op06_OAM[op06_index] |= (size << (op06_offset + 1));
   op06_offset += 2;

   if (op06_offset == 8)
   {
      /* move to next byte in buffer */
      op06_offset = 0;
      op06_index++;
   }
}

void DSP4_Op01(void)
{
   int16_t plane;
   int16_t index, lcv;
   int16_t py_dy, px_dx;
   int16_t y_out, x_out;
   uint16_t command;
   DSP4.waiting4command = false;

   switch (DSP4_Logic) /* op flow control */
   {
   case 1:
      goto resume1;
      break;
   case 2:
      goto resume2;
      break;
   }

   /*
    * process initial inputs
    */

   /* sort inputs */
   project_focaly = DSP4_READ_WORD(0x02);
   raster = DSP4_READ_WORD(0x04);
   viewport_top = DSP4_READ_WORD(0x06);
   project_y = DSP4_READ_WORD(0x08);
   viewport_bottom = DSP4_READ_WORD(0x0a);
   project_x1low = DSP4_READ_WORD(0x0c);
   project_focalx = DSP4_READ_WORD(0x0e);
   project_centerx = DSP4_READ_WORD(0x10);
   project_ptr = DSP4_READ_WORD(0x12);
   project_pitchylow = DSP4_READ_WORD(0x16);
   project_pitchy = DSP4_READ_WORD(0x18);
   project_pitchxlow = DSP4_READ_WORD(0x1a);
   project_pitchx = DSP4_READ_WORD(0x1c);
   far_plane = DSP4_READ_WORD(0x1e);
   project_y1low = DSP4_READ_WORD(0x22);

   /* pre-compute */
   view_plane = PLANE_START;

   /* find starting projection points */
   project_x1 = project_focalx;
   project_y -= viewport_bottom;
   project_x = project_centerx + project_x1;

   /* multi-op storage */
   multi_index1 = 0;
   multi_index2 = 0;

   /*
    * command check
    */

   do
   {
      /* scan next command */
      DSP4.in_count = 2;
      DSP4_WAIT(1);

resume1:
      /* inspect input */
      command = DSP4_READ_WORD(0);

      /* check for termination */
      if(command == 0x8000)
         break;

      /* already have 2 bytes in queue */
      DSP4.in_index = 2;
      DSP4.in_count = 8;
      DSP4_WAIT(2);

      /* process one iteration of projection */

      /* inspect inputs */

resume2:
      plane = DSP4_READ_WORD(0);
      px_dx = 0;

      /* ignore invalid data */
      if((uint16_t) plane == 0x8001)
         continue;

      /* one-time init */
      if (far_plane)
      {
         /* setup final parameters */
         project_focalx += plane;
         project_x1 = project_focalx;
         project_y1 = project_focaly;
         plane = far_plane;
         far_plane = 0;
      }

      /* use proportional triangles to project new coords */
      project_x2 = project_focalx * plane / view_plane;
      project_y2 = project_focaly * plane / view_plane;

      /* quadratic regression (rough) */
      if (project_focaly >= -0x0f)
         py_dy = (int16_t)(project_focaly * project_focaly * -0.20533553 - 1.08330005 * project_focaly - 69.61094639);
      else
         py_dy = (int16_t)(project_focaly * project_focaly * -0.000657035759 - 1.07629051 * project_focaly - 65.69315963);

      /* approximate # of raster lines */
      segments = ABS(project_y2 - project_y1);

      /* prevent overdraw */
      if(project_y2 >= raster)
         segments = 0;
      else
         raster = project_y2;

      /* don't draw outside the window */
      if(project_y2 < viewport_top)
         segments = 0;

      /* project new positions */
      if (segments > 0)
         px_dx = ((project_x2 - project_x1) << 8) / segments; /* interpolate between projected points */

      /* prepare output */
      DSP4.out_count = 8 + 2 + 6 * segments;

      /* pre-block data */
      DSP4_WRITE_WORD(0, project_focalx);
      DSP4_WRITE_WORD(2, project_x2);
      DSP4_WRITE_WORD(4, project_focaly);
      DSP4_WRITE_WORD(6, project_y2);
      DSP4_WRITE_WORD(8, segments);

      index = 10;

      for (lcv = 0; lcv < segments; lcv++) /* iterate through each point */
      {
         /* step through the projected line */
         y_out = project_y + ((py_dy * lcv) >> 8);
         x_out = project_x + ((px_dx * lcv) >> 8);

         /* data */
         DSP4_WRITE_WORD(index + 0, project_ptr);
         DSP4_WRITE_WORD(index + 2, y_out);
         DSP4_WRITE_WORD(index + 4, x_out);
         index += 6;

         /* post-update */
         project_ptr -= 4;
      }

      /* post-update */
      project_y += ((py_dy * lcv) >> 8);
      project_x += ((px_dx * lcv) >> 8);

      if (segments > 0) /* new positions */
      {
         project_x1 = project_x2;
         project_y1 = project_y2;

         /* multi-op storage */
         multi_focaly[multi_index2++] = project_focaly;
         multi_farplane[1] = plane;
         multi_raster[1] = project_y1 - 1;
      }

      /* update projection points */
      project_pitchy += (int8_t)DSP4.parameters[3];
      project_pitchx += (int8_t)DSP4.parameters[5];

      project_focaly += project_pitchy;
      project_focalx += project_pitchx;
   } while (1);

   /* terminate op */
   DSP4.waiting4command = true;
   DSP4.out_count = 0;
}

void DSP4_Op07(void)
{
   uint16_t command;
   int16_t plane;
   int16_t index, lcv;
   int16_t y_out, x_out;
   int16_t py_dy, px_dx;

   DSP4.waiting4command = false;

   /* op flow control */
   switch (DSP4_Logic)
   {
   case 1:
      goto resume1;
      break;
   case 2:
      goto resume2;
      break;
   }

   /* sort inputs */

   project_focaly = DSP4_READ_WORD(0x02);
   raster = DSP4_READ_WORD(0x04);
   viewport_top = DSP4_READ_WORD(0x06);
   project_y = DSP4_READ_WORD(0x08);
   viewport_bottom = DSP4_READ_WORD(0x0a);
   project_x1low = DSP4_READ_WORD(0x0c);
   project_x1 = DSP4_READ_WORD(0x0e);
   project_centerx = DSP4_READ_WORD(0x10);
   project_ptr = DSP4_READ_WORD(0x12);

   /* pre-compute */
   view_plane = PLANE_START;

   /* find projection targets */
   project_y1 = project_focaly;
   project_y -= viewport_bottom;
   project_x = project_centerx + project_x1;

   /* multi-op storage */
   multi_index2 = 0;

   /* command check */

   do
   {
      /* scan next command */
      DSP4.in_count = 2;
      DSP4_WAIT(1);

resume1:
      /* inspect input */
      command = DSP4_READ_WORD(0);

      /* check for opcode termination */
      if(command == 0x8000)
         break;

      /* already have 2 bytes in queue */
      DSP4.in_index = 2;
      DSP4.in_count = 12;
      DSP4_WAIT(2);

      /* process one loop of projection */

resume2:
      px_dx = 0;

      /* inspect inputs */
      plane = DSP4_READ_WORD(0);
      project_y2 = DSP4_READ_WORD(2);
      project_x2 = DSP4_READ_WORD(6);

      /* ignore invalid data */
      if((uint16_t) plane == 0x8001)
         continue;

      /* multi-op storage */
      project_focaly = multi_focaly[multi_index2];

      /* quadratic regression (rough) */
      if (project_focaly >= -0x0f)
         py_dy = (int16_t)(project_focaly * project_focaly * -0.20533553 - 1.08330005 * project_focaly - 69.61094639);
      else
         py_dy = (int16_t)(project_focaly * project_focaly * -0.000657035759 - 1.07629051 * project_focaly - 65.69315963);

      /* approximate # of raster lines */
      segments = ABS(project_y2 - project_y1);

      /* prevent overdraw */
      if(project_y2 >= raster)
         segments = 0;
      else
         raster = project_y2;

      /* don't draw outside the window */
      if(project_y2 < viewport_top)
         segments = 0;

      /* project new positions */
      if (segments > 0)
      {
         /* interpolate between projected points */
         px_dx = ((project_x2 - project_x1) << 8) / segments;
      }

      /* prepare pre-output */
      DSP4.out_count = 4 + 2 + 6 * segments;

      DSP4_WRITE_WORD(0, project_x2);
      DSP4_WRITE_WORD(2, project_y2);
      DSP4_WRITE_WORD(4, segments);

      index = 6;
      for (lcv = 0; lcv < segments; lcv++)
      {
         /* pre-compute */
         y_out = project_y + ((py_dy * lcv) >> 8);
         x_out = project_x + ((px_dx * lcv) >> 8);

         /* data */
         DSP4_WRITE_WORD(index + 0, project_ptr);
         DSP4_WRITE_WORD(index + 2, y_out);
         DSP4_WRITE_WORD(index + 4, x_out);
         index += 6;

         /* post-update */
         project_ptr -= 4;
      }

      /* update internal variables */
      project_y += ((py_dy * lcv) >> 8);
      project_x += ((px_dx * lcv) >> 8);

      /* new positions */
      if (segments > 0)
      {
         project_x1 = project_x2;
         project_y1 = project_y2;

         /* multi-op storage */
         multi_index2++;
      }
   } while (1);

   DSP4.waiting4command = true;
   DSP4.out_count = 0;
}

void DSP4_Op08(void)
{
   uint16_t command;
   /* used in envelope shaping */
   int16_t x1_final;
   int16_t x2_final;
   int16_t plane, x_left, y_left, x_right, y_right;
   int16_t envelope1, envelope2;
   DSP4.waiting4command = false;

   /* op flow control */
   switch (DSP4_Logic)
   {
   case 1:
      goto resume1;
      break;
   case 2:
      goto resume2;
      break;
   }

   /* process initial inputs */

   /* clip values */
   path_clipRight[0] = DSP4_READ_WORD(0x00);
   path_clipRight[1] = DSP4_READ_WORD(0x02);
   path_clipRight[2] = DSP4_READ_WORD(0x04);
   path_clipRight[3] = DSP4_READ_WORD(0x06);

   path_clipLeft[0] = DSP4_READ_WORD(0x08);
   path_clipLeft[1] = DSP4_READ_WORD(0x0a);
   path_clipLeft[2] = DSP4_READ_WORD(0x0c);
   path_clipLeft[3] = DSP4_READ_WORD(0x0e);

   /* path positions */
   path_pos[0] = DSP4_READ_WORD(0x20);
   path_pos[1] = DSP4_READ_WORD(0x22);
   path_pos[2] = DSP4_READ_WORD(0x24);
   path_pos[3] = DSP4_READ_WORD(0x26);

   /* data locations */
   path_ptr[0] = DSP4_READ_WORD(0x28);
   path_ptr[1] = DSP4_READ_WORD(0x2a);
   path_ptr[2] = DSP4_READ_WORD(0x2c);
   path_ptr[3] = DSP4_READ_WORD(0x2e);

   /* project_y1 lines */
   path_raster[0] = DSP4_READ_WORD(0x30);
   path_raster[1] = DSP4_READ_WORD(0x32);
   path_raster[2] = DSP4_READ_WORD(0x34);
   path_raster[3] = DSP4_READ_WORD(0x36);

   /* viewport_top */
   path_top[0] = DSP4_READ_WORD(0x38);
   path_top[1] = DSP4_READ_WORD(0x3a);
   path_top[2] = DSP4_READ_WORD(0x3c);
   path_top[3] = DSP4_READ_WORD(0x3e);

   /* unknown (constants) */

   view_plane = PLANE_START;

   /* command check */

   do
   {
      /* scan next command */
      DSP4.in_count = 2;
      DSP4_WAIT(1);

resume1:
      /* inspect input */
      command = DSP4_READ_WORD(0);

      /* terminate op */
      if(command == 0x8000)
         break;

      /* already have 2 bytes in queue */
      DSP4.in_index = 2;
      DSP4.in_count = 18;
      DSP4_WAIT(2);

resume2:
      /* projection begins */

      /* look at guidelines */
      plane = DSP4_READ_WORD(0x00);
      x_left = DSP4_READ_WORD(0x02);
      y_left = DSP4_READ_WORD(0x04);
      x_right = DSP4_READ_WORD(0x06);
      y_right = DSP4_READ_WORD(0x08);

      /* envelope guidelines (one frame only) */
      envelope1 = DSP4_READ_WORD(0x0a);
      envelope2 = DSP4_READ_WORD(0x0c);

      /* ignore invalid data */
      if((uint16_t) plane == 0x8001)
         continue;

      /* first init */
      if (plane == 0x7fff)
      {
         int32_t pos1, pos2;

         /* initialize projection */
         path_x[0] = x_left;
         path_x[1] = x_right;

         path_y[0] = y_left;
         path_y[1] = y_right;

         /* update coordinates */
         path_pos[0] -= x_left;
         path_pos[1] -= x_left;
         path_pos[2] -= x_right;
         path_pos[3] -= x_right;

         pos1 = path_pos[0] + envelope1;
         pos2 = path_pos[1] + envelope2;

         /* clip offscreen data */
         if(pos1 < path_clipLeft[0])
            pos1 = path_clipLeft[0];
         if(pos1 > path_clipRight[0])
            pos1 = path_clipRight[0];
         if(pos2 < path_clipLeft[1])
            pos2 = path_clipLeft[1];
         if(pos2 > path_clipRight[1])
            pos2 = path_clipRight[1];

         path_plane[0] = plane;
         path_plane[1] = plane;

         /* initial output */
         DSP4.out_count = 2;
         DSP4.output[0] = pos1 & 0xFF;
         DSP4.output[1] = pos2 & 0xFF;
      }
      /* proceed with projection */
      else
      {
         int16_t index = 0, lcv;
         int16_t left_inc = 0, right_inc = 0;
         int16_t dx1 = 0, dx2 = 0, dx3, dx4;

         /* # segments to traverse */
         segments = ABS(y_left - path_y[0]);

         /* prevent overdraw */
         if(y_left >= path_raster[0])
            segments = 0;
         else
            path_raster[0] = y_left;

         /* don't draw outside the window */
         if(path_raster[0] < path_top[0])
            segments = 0;

         /* proceed if visibility rules apply */
         if (segments > 0)
         {
            /* use previous data */
            dx1 = (envelope1 * path_plane[0] / view_plane);
            dx2 = (envelope2 * path_plane[0] / view_plane);

            /* use temporary envelope pitch (this frame only) */
            dx3 = (envelope1 * plane / view_plane);
            dx4 = (envelope2 * plane / view_plane);

            /* project new shapes (left side) */
            x1_final = x_left + dx1;
            x2_final = path_x[0] + dx3;

            /* interpolate between projected points with shaping */
            left_inc = ((x2_final - x1_final) << 8) / segments;

            /* project new shapes (right side) */
            x1_final = x_left + dx2;
            x2_final = path_x[0] + dx4;

            /* interpolate between projected points with shaping */
            right_inc = ((x2_final - x1_final) << 8) / segments;
            path_plane[0] = plane;
         }

         /* zone 1 */
         DSP4.out_count = (2 + 4 * segments);
         DSP4_WRITE_WORD(index, segments);
         index += 2;

         for (lcv = 1; lcv <= segments; lcv++)
         {
            int16_t pos1, pos2;

            /* pre-compute */
            pos1 = path_pos[0] + ((left_inc * lcv) >> 8) + dx1;
            pos2 = path_pos[1] + ((right_inc * lcv) >> 8) + dx2;

            /* clip offscreen data */
            if(pos1 < path_clipLeft[0])
               pos1 = path_clipLeft[0];
            if(pos1 > path_clipRight[0])
               pos1 = path_clipRight[0];
            if(pos2 < path_clipLeft[1])
               pos2 = path_clipLeft[1];
            if(pos2 > path_clipRight[1])
               pos2 = path_clipRight[1];

            /* data */
            DSP4_WRITE_WORD(index, path_ptr[0]);
            index += 2;
            DSP4.output[index++] = pos1 & 0xFF;
            DSP4.output[index++] = pos2 & 0xFF;

            /* post-update */
            path_ptr[0] -= 4;
            path_ptr[1] -= 4;
         }
         lcv--;

         if (segments > 0)
         {
            /* project points w/out the envelopes */
            int16_t inc = ((path_x[0] - x_left) << 8) / segments;

            /* post-store */
            path_pos[0] += ((inc * lcv) >> 8);
            path_pos[1] += ((inc * lcv) >> 8);

            path_x[0] = x_left;
            path_y[0] = y_left;
         }

         /* zone 2 */
         segments = ABS(y_right - path_y[1]);

         /* prevent overdraw */
         if(y_right >= path_raster[2])
            segments = 0;
         else path_raster[2] = y_right;

         /* don't draw outside the window */
         if(path_raster[2] < path_top[2])
            segments = 0;

         /* proceed if visibility rules apply */
         if (segments > 0)
         {
            /* use previous data */
            dx1 = (envelope1 * path_plane[1] / view_plane);
            dx2 = (envelope2 * path_plane[1] / view_plane);

            /* use temporary envelope pitch (this frame only) */
            dx3 = (envelope1 * plane / view_plane);
            dx4 = (envelope2 * plane / view_plane);

            /* project new shapes (left side) */
            x1_final = x_left + dx1;
            x2_final = path_x[1] + dx3;

            /* interpolate between projected points with shaping */
            left_inc = ((x2_final - x1_final) << 8) / segments;

            /* project new shapes (right side) */
            x1_final = x_left + dx2;
            x2_final = path_x[1] + dx4;

            /* interpolate between projected points with shaping */
            right_inc = ((x2_final - x1_final) << 8) / segments;

            path_plane[1] = plane;
         }

         /* write out results */
         DSP4.out_count += (2 + 4 * segments);
         DSP4_WRITE_WORD(index, segments);
         index += 2;

         for (lcv = 1; lcv <= segments; lcv++)
         {
            int16_t pos1, pos2;

            /* pre-compute */
            pos1 = path_pos[2] + ((left_inc * lcv) >> 8) + dx1;
            pos2 = path_pos[3] + ((right_inc * lcv) >> 8) + dx2;

            /* clip offscreen data */
            if(pos1 < path_clipLeft[2])
               pos1 = path_clipLeft[2];
            if(pos1 > path_clipRight[2])
               pos1 = path_clipRight[2];
            if(pos2 < path_clipLeft[3])
               pos2 = path_clipLeft[3];
            if(pos2 > path_clipRight[3])
               pos2 = path_clipRight[3];

            /* data */
            DSP4_WRITE_WORD(index, path_ptr[2]);
            index += 2;
            DSP4.output[index++] = pos1 & 0xFF;
            DSP4.output[index++] = pos2 & 0xFF;

            /* post-update */
            path_ptr[2] -= 4;
            path_ptr[3] -= 4;
         }
         lcv--;

         if (segments > 0)
         {
            /* project points w/out the envelopes */
            int16_t inc = ((path_x[1] - x_right) << 8) / segments;

            /* post-store */
            path_pos[2] += ((inc * lcv) >> 8);
            path_pos[3] += ((inc * lcv) >> 8);

            path_x[1] = x_right;
            path_y[1] = y_right;
         }
      }
   } while (1);

   DSP4.waiting4command = true;
   DSP4.out_count = 2;
   DSP4_WRITE_WORD(0, 0);
}

void DSP4_Op0D(void)
{
   uint16_t command;
   /* inspect inputs */
   int16_t plane;
   int16_t index, lcv;
   int16_t py_dy, px_dx;
   int16_t y_out, x_out;

   DSP4.waiting4command = false;

   /* op flow control */
   switch (DSP4_Logic)
   {
   case 1:
      goto resume1;
      break;
   case 2:
      goto resume2;
      break;
   }

   /* process initial inputs */

   /* sort inputs */
   project_focaly = DSP4_READ_WORD(0x02);
   raster = DSP4_READ_WORD(0x04);
   viewport_top = DSP4_READ_WORD(0x06);
   project_y = DSP4_READ_WORD(0x08);
   viewport_bottom = DSP4_READ_WORD(0x0a);
   project_x1low = DSP4_READ_WORD(0x0c);
   project_x1 = DSP4_READ_WORD(0x0e);
   project_focalx = DSP4_READ_WORD(0x0e);
   project_centerx = DSP4_READ_WORD(0x10);
   project_ptr = DSP4_READ_WORD(0x12);
   project_pitchylow = DSP4_READ_WORD(0x16);
   project_pitchy = DSP4_READ_WORD(0x18);
   project_pitchxlow = DSP4_READ_WORD(0x1a);
   project_pitchx = DSP4_READ_WORD(0x1c);
   far_plane = DSP4_READ_WORD(0x1e);

   /* multi-op storage */
   multi_index1++;
   multi_index1 %= 4;

   /* remap 0D->09 window data ahead of time */
   /* index starts at 1-3,0 */
   /* Op0D: BL,TL,BR,TR */
   /* Op09: TL,TR,BL,BR (1,2,3,0) */
   switch (multi_index1)
   {
   case 1:
      multi_index2 = 3;
      break;
   case 2:
      multi_index2 = 1;
      break;
   case 3:
      multi_index2 = 0;
      break;
   case 0:
      multi_index2 = 2;
      break;
   }

   /* pre-compute */
   view_plane = PLANE_START;

   /* figure out projection data */
   project_y -= viewport_bottom;
   project_x = project_centerx + project_x1;

   /* command check */

   do
   {
      /* scan next command */
      DSP4.in_count = 2;
      DSP4_WAIT(1);

resume1:
      /* inspect input */
      command = DSP4_READ_WORD(0);

      /* terminate op */
      if(command == 0x8000)
         break;

      /* already have 2 bytes in queue */
      DSP4.in_index = 2;
      DSP4.in_count = 8;
      DSP4_WAIT(2);

      /* project section of the track */

resume2:
      plane = DSP4_READ_WORD(0);
      px_dx = 0;


      /* ignore invalid data */
      if((uint16_t) plane == 0x8001)
         continue;

      /* one-time init */
      if (far_plane)
      {
         /* setup final data */
         project_x1 = project_focalx;
         project_y1 = project_focaly;
         plane = far_plane;
         far_plane = 0;
      }

      /* use proportional triangles to project new coords */
      project_x2 = project_focalx * plane / view_plane;
      project_y2 = project_focaly * plane / view_plane;

      /* quadratic regression (rough) */
      if (project_focaly >= -0x0f)
         py_dy = (int16_t)(project_focaly * project_focaly * -0.20533553 - 1.08330005 * project_focaly - 69.61094639);
      else
         py_dy = (int16_t)(project_focaly * project_focaly * -0.000657035759 - 1.07629051 * project_focaly - 65.69315963);

      /* approximate # of raster lines */
      segments = ABS(project_y2 - project_y1);

      /* prevent overdraw */
      if(project_y2 >= raster)
         segments = 0;
      else
         raster = project_y2;

      /* don't draw outside the window */
      if(project_y2 < viewport_top)
         segments = 0;

      /* project new positions */
      if (segments > 0)
      {
         /* interpolate between projected points */
         px_dx = ((project_x2 - project_x1) << 8) / segments;
      }

      /* prepare output */
      DSP4.out_count = 8 + 2 + 6 * segments;
      DSP4_WRITE_WORD(0, project_focalx);
      DSP4_WRITE_WORD(2, project_x2);
      DSP4_WRITE_WORD(4, project_focaly);
      DSP4_WRITE_WORD(6, project_y2);
      DSP4_WRITE_WORD(8, segments);
      index = 10;

      for (lcv = 0; lcv < segments; lcv++) /* iterate through each point */
      {
         /* step through the projected line */
         y_out = project_y + ((py_dy * lcv) >> 8);
         x_out = project_x + ((px_dx * lcv) >> 8);

         /* data */
         DSP4_WRITE_WORD(index + 0, project_ptr);
         DSP4_WRITE_WORD(index + 2, y_out);
         DSP4_WRITE_WORD(index + 4, x_out);
         index += 6;

         /* post-update */
         project_ptr -= 4;
      }

      /* post-update */
      project_y += ((py_dy * lcv) >> 8);
      project_x += ((px_dx * lcv) >> 8);

      if (segments > 0)
      {
         project_x1 = project_x2;
         project_y1 = project_y2;

         /* multi-op storage */
         multi_farplane[multi_index2] = plane;
         multi_raster[multi_index2] = project_y1;
      }

      /* update focal projection points */
      project_pitchy += (int8_t)DSP4.parameters[3];
      project_pitchx += (int8_t)DSP4.parameters[5];

      project_focaly += project_pitchy;
      project_focalx += project_pitchx;
   } while (1);

   DSP4.waiting4command = true;
   DSP4.out_count = 0;
}

void DSP4_Op09(void)
{
   uint16_t command;
   bool clip;
   int16_t sp_x, sp_y, sp_oam, sp_msb;
   int16_t sp_dx, sp_dy;

   DSP4.waiting4command = false;

   /* op flow control */
   switch (DSP4_Logic)
   {
   case 1:
      goto resume1;
      break;
   case 2:
      goto resume2;
      break;
   case 3:
      goto resume3;
      break;
   case 4:
      goto resume4;
      break;
   case 5:
      goto resume5;
      break;
   case 6:
      goto resume6;
      break;
   case 7:
      goto resume7;
      break;
   }

   /* process initial inputs */

   /* grab screen information */
   view_plane = PLANE_START;
   center_x = DSP4_READ_WORD(0x00);
   center_y = DSP4_READ_WORD(0x02);
   viewport_left = DSP4_READ_WORD(0x06);
   viewport_right = DSP4_READ_WORD(0x08);
   viewport_top = DSP4_READ_WORD(0x0a);
   viewport_bottom = DSP4_READ_WORD(0x0c);

   /* expand viewport dimensions */
   viewport_left -= 8;

   /* cycle through viewport window data */
   multi_index1++;
   multi_index1 %= 4;

   /* convert track line to the window region */
   project_y2 = center_y + multi_raster[multi_index1] * (viewport_bottom - center_y) / (0x33 - 0);
   if (!op09_mode)
      project_y2 -= 2;

   goto no_sprite;

   do
   {
      /* check for new sprites */
      do
      {
         uint16_t second;

         DSP4.in_count = 4;
         DSP4.in_index = 2;
         DSP4_WAIT(1);

resume1:
         /* try to classify sprite */
         second = DSP4_READ_WORD(2);

         /* op termination */
         if(second == 0x8000)
            goto terminate;

         second >>= 8;
         sprite_type = 0;

         /* vehicle sprite */
         if (second == 0x90)
         {
            sprite_type = 1;
            break;
         }
         /* terrain sprite */
         else if (second != 0)
         {
            sprite_type = 2;
            break;
         }

no_sprite:
         /* no sprite. try again */
         DSP4.in_count = 2;
         DSP4_WAIT(2);

resume2:;
      } while (1);

      /* process projection information */

sprite_found:
      /* vehicle sprite */
      if (sprite_type == 1)
      {
         int16_t plane;
         int16_t car_left, car_right;
         int16_t focal_back;
         int32_t height;

         /* we already have 4 bytes we want */
         DSP4.in_count = 6 + 12;
         DSP4.in_index = 4;
         DSP4_WAIT(3);

resume3:
         /* filter inputs */
         project_y1 = DSP4_READ_WORD(0x00);
         focal_back = DSP4_READ_WORD(0x06);
         car_left = DSP4_READ_WORD(0x0c);
         plane = DSP4_READ_WORD(0x0e);
         car_right = DSP4_READ_WORD(0x10);

         /* calculate car's x-center */
         project_focalx = car_right - car_left;

         /* determine how far into the screen to project */
         project_focaly = focal_back;
         project_x = project_focalx * plane / view_plane;
         segments = 0x33 - project_focaly * plane / view_plane;
         far_plane = plane;

         /* prepare memory */
         sprite_x = center_x + project_x;
         sprite_y = viewport_bottom - segments;
         far_plane = plane;

         /* make the car's x-center available */
         DSP4.out_count = 2;
         DSP4_WRITE_WORD(0, project_focalx);

         /* grab a few remaining vehicle values */
         DSP4.in_count = 4;

         DSP4_WAIT(4);

resume4: /* store final values */
         height = DSP4_READ_WORD(0);
         sprite_offset = DSP4_READ_WORD(2);

         /* vertical lift factor */
         sprite_y += height;
      }
      else if (sprite_type == 2) /* terrain sprite */
      {
         int16_t plane;

         /* we already have 4 bytes we want */
         DSP4.in_count = 6 + 6 + 2;
         DSP4.in_index = 4;
         DSP4_WAIT(5);

resume5:
         /* sort loop inputs */
         project_y1 = DSP4_READ_WORD(0x00);
         plane = DSP4_READ_WORD(0x02);
         project_centerx = DSP4_READ_WORD(0x04);
         project_focalx = DSP4_READ_WORD(0x08);
         project_focaly = DSP4_READ_WORD(0x0a);
         sprite_offset = DSP4_READ_WORD(0x0c);

         /* determine distances into virtual world */
         segments = 0x33 - project_y1;
         project_x = project_focalx * plane / view_plane;
         project_y = project_focaly * plane / view_plane;

         /* prepare memory */
         sprite_x = center_x + project_x - project_centerx;
         sprite_y = viewport_bottom - segments + project_y;
         far_plane = plane;
      }

      /* default sprite size: 16x16 */
      sprite_size = true;

      /* convert tile data to OAM */

      do
      {
         DSP4.in_count = 2;
         DSP4_WAIT(6);

resume6:
         command = DSP4_READ_WORD(0);

         /* opcode termination */
         if(command == 0x8000)
            goto terminate;

         /* toggle sprite size */
         if (command == 0x0000)
         {
            sprite_size = !sprite_size;
            continue;
         }

         /* new sprite information */
         command >>= 8;
         if (command != 0x20 && command != 0x40 && command != 0x60 && command != 0xa0 && command != 0xc0 && command != 0xe0)
            break;

         DSP4.in_count = 6;
         DSP4.in_index = 2;
         DSP4_WAIT(7);

         /* process tile data */

resume7:
         /* sprite deltas */
         sp_dy = DSP4_READ_WORD(2);
         sp_dx = DSP4_READ_WORD(4);

         /* update coordinates */
         sp_y = sprite_y + sp_dy;
         sp_x = sprite_x + sp_dx;

         /* reject points outside the clipping window */
         clip = false;
         if(sp_x < viewport_left || sp_x > viewport_right)
            clip = true;
         if(sp_y < viewport_top || sp_y > viewport_bottom)
            clip = true;

         /* track depth sorting */
         if(far_plane <= multi_farplane[multi_index1] && sp_y >= project_y2)
            clip = true;

         /* don't draw offscreen coordinates */
         DSP4.out_count = 0;
         if (!clip)
         {
            int16_t out_index = 0;
            int16_t offset = DSP4_READ_WORD(0);

            /* update sprite nametable/attribute information */
            sp_oam = sprite_offset + offset;
            sp_msb = (sp_x < 0 || sp_x > 255);

            /* emit transparency information */
            if((sprite_offset & 0x08) && ((sprite_type == 1 && sp_y >= 0xcc) || (sprite_type == 2 && sp_y >= 0xbb)))
            {
               DSP4.out_count = 6;

               /* one block of OAM data */
               DSP4_WRITE_WORD(0, 1);

               /* OAM: x,y,tile,no attr */
               DSP4.output[2] = sp_x & 0xFF;
               DSP4.output[3] = (sp_y + 6) & 0xFF;
               DSP4_WRITE_WORD(4, 0xEE);
               out_index = 6;

               /* OAM: size,msb data */
               DSP4_Op06(sprite_size, (int8_t) sp_msb);
            }

            /* normal data */
            DSP4.out_count += 8;

            /* one block of OAM data */
            DSP4_WRITE_WORD(out_index + 0, 1);

            /* OAM: x,y,tile,attr */
            DSP4.output[out_index + 2] = sp_x & 0xFF;
            DSP4.output[out_index + 3] = sp_y & 0xFF;
            DSP4_WRITE_WORD(out_index + 4, sp_oam);

            /* no following OAM data */
            DSP4_WRITE_WORD(out_index + 6, 0);

            /* OAM: size,msb data */
            DSP4_Op06(sprite_size, (int8_t) sp_msb);
         }

         /* no sprite information */
         if (DSP4.out_count == 0)
         {
            DSP4.out_count = 2;
            DSP4_WRITE_WORD(0, 0);
         }
      } while (1);

      /* special cases: plane == 0x0000 */

      /* special vehicle case */
      if (command == 0x90)
      {
         sprite_type = 1;

         /* shift bytes */
         DSP4.parameters[2] = DSP4.parameters[0];
         DSP4.parameters[3] = DSP4.parameters[1];
         DSP4.parameters[0] = 0;
         DSP4.parameters[1] = 0;

         goto sprite_found;
      }
      else if (command != 0x00 && command != 0xff) /* special terrain case */
      {
         sprite_type = 2;

         /* shift bytes */
         DSP4.parameters[2] = DSP4.parameters[0];
         DSP4.parameters[3] = DSP4.parameters[1];
         DSP4.parameters[0] = 0;
         DSP4.parameters[1] = 0;

         goto sprite_found;
      }
   } while (1);

terminate:
   DSP4.waiting4command = true;
   DSP4.out_count = 0;
}
