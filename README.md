# Spiral Pattern util

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This is an example how to create a spiral "pattern" with Fusion 360 Api.
# Script
- Script is written in C++ architecture
- This script is supported for Windows only

# Run Example
1. Install Fusion 360
2. Find this folder `c:\Users\<user_name>\AppData\Roaming\Autodesk\Autodesk Fusion 360\API\Scripts\`
3. Clone repository
4. Run Fusion 360
5. Go to Utillities->ADD-INS
6. Select "SpiralPattern"
7. Run this script

# Workflow
1. Select an axis (Work axis/Cylinder face/Circle edge)
2. Select objects you want to copy with rotation and distance
3. Define parameters
4. Press OK

Hint: in case that pater is rotating or buildin got oposide direction then you can use negative angle and distance values

# Result in Fusion 360
The workflow above creates copies of selected occurrences and move them to possition according to specified parameters. This script do not create feature which you can edit. 