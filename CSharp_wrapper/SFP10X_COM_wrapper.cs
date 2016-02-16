/*
 
 Copyright 2015-2016 Sendyne Corp., New York, USA
 http://www.sendyne.com
 
 The C# wrapper utilizes the compiled SFP10X_COM.dll C library for 
 FTDI-based serial communication with Sendyne's SFP products.
 
 Authors:
    Damian Glinojecki (Sendyne Corp.)
    Nicolas Clauvelin (Sendyne Corp.)
 
 File:
    SFP10X_COM_wrapper.cs
 
 THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SFP10X_wrapper
{
    // SFP communications library wrapper
    class SFP10X_COM_wrapper
    {
        // SFP Device Struct recreated with a C# native void pointer
        public struct SFPDevice
        {
            IntPtr sfp_handle;
            int sfp_device_num;
        };
        
        // Initialize  
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Initialize(int device_num, ref SFPDevice sfp_dev);

        // ChangeTimeout 
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ChangeTimeout(ref SFPDevice device, int time_ms);

        // ReadRegister  
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ReadRegister(ref SFPDevice device,
                                                    byte SFP_reg_address,
                                                    byte number_of_bytes,
                                                    [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)] byte[] data);

        // ReadSignedRegister  
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ReadSignedRegister(ref SFPDevice device,
                                                    byte SFP_reg_address,
                                                    byte number_of_bytes,
                                                    ref long data);

        // WriteRegister
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte WriteRegister(ref SFPDevice device,
                                                    byte SFP_reg_address,
                                                    byte number_of_bytes,
                                                    [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)] byte[] data);

        // ChangeBaudRate
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ChangeBaudRate(ref SFPDevice device, byte baud_rate);

        // ChangeOnlyHostBaudRate
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ChangeOnlyHostBaudRate(ref SFPDevice device, byte baud_rate);

        // ClosePort  
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ClosePort(ref SFPDevice device);

        // GetFTDIDeviceCount 
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetFTDIDeviceCount();

        // GetFTDIDeviceInfo 
        // Note: StringBuilder is used in order to simplify displaying the returned string
        [DllImport("SFP10X_COM.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte GetFTDIDeviceInfo(int device_num, StringBuilder buffer);
    }
}
