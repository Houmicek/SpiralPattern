
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>

#include <list>

#if defined(_WINDOWS) || defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

// Globals
Ptr<Application> _app;
Ptr<UserInterface> _ui;
std::string _commandId = "SpiralPatternCmd";
Ptr<std::string> _errMessage;
std::string _units = "";
std::list<Ptr<Base>> _occurrences;
Ptr<Point3D> _point;
Ptr<Vector3D> _normal;

// Input declarations.
Ptr<SelectionCommandInput> _objectSelection;
Ptr<SelectionCommandInput> _axisSelection;
Ptr<IntegerSliderCommandInput> _number;
Ptr<ValueCommandInput> _height;
Ptr<ValueCommandInput> _distance;
Ptr<ValueCommandInput> _angle;


bool getCommandInputValue(Ptr<CommandInput> commandInput, std::string unitType, double* value);
bool is_digits(const std::string& str);
bool getSelectedComponents(Ptr<SelectionCommandInput> selectionInput);
bool getSelectedAxis(Ptr<SelectionCommandInput>selectionInput);

bool checkReturn(Ptr<Base> returnObj)
{
    if (returnObj)
        return true;
    else
        if (_app && _ui)
        {
            std::string errDesc;
            _app->getLastError(&errDesc);
            _ui->messageBox(errDesc);
            return false;
        }
        else
            return false;
}

class GearCommandInputChangedHandler : public adsk::core::InputChangedEventHandler
{
public:
    void notify(const Ptr<InputChangedEventArgs>& eventArgs) override
    {
        Ptr<CommandInput> changedInput = eventArgs->input();
        double occurrencesCount = 0;
        double occurrencesHeight = 0;
        double occurrencesLevelDistance = 0;

        //read data from Command UI
        occurrencesCount = _number->valueOne();

        double value;
        if (getCommandInputValue(_height, "m", &value))
        {
            occurrencesHeight = value;
        }

        if (getCommandInputValue(_distance, "mm", &value))
        {
            occurrencesLevelDistance = value;
        }

        //Update data 
        if (changedInput->id() == _commandId + "_number" || changedInput->id() == _commandId + "_height")
        {
            _distance->value(occurrencesHeight / (occurrencesCount - 1));
        }
        else if (changedInput->id() == _commandId + "_distance")
        {
            _height->value(occurrencesLevelDistance * (occurrencesCount - 1));
        }
    }
} _cmdInputChanged;

// Event handler for the execute event.
class SpiralPatternCommandExecuteEventHandler : public adsk::core::CommandEventHandler
{
public:
    void notify(const Ptr<CommandEventArgs>& eventArgs) override
    {
        _point = nullptr;
        _normal = nullptr;

        //Get selected bodies
        _occurrences.clear();
        if (!getSelectedComponents(_objectSelection)) {
            _ui->messageBox("Something went wrong with Component selection!");
            return;
        }

        if (_occurrences.size() == 0) {
            _ui->messageBox("No Components were selected!");
            return;
        }

        if (!getSelectedAxis(_axisSelection)) {
            _ui->messageBox("Something went wrong with Axis selection!");
            return;
        }

        if (!_point || !_normal) {
            _ui->messageBox("No Axis was selected!");
            return;
        }
    }
} _spiralPatternCommandExecute;

class SpiralPatternCommandValidateInputsEventHandler : public adsk::core::ValidateInputsEventHandler
{
public:
    void notify(const Ptr<ValidateInputsEventArgs>& eventArgs) override
    {

    }
} _cmdValidateInputs;

class SpiralPatternGearCommandDestroyEventHandler : public adsk::core::CommandEventHandler
{
public:
    void notify(const Ptr<CommandEventArgs>& eventArgs) override
    {
        // Terminate the script since the command has finished.
        adsk::terminate();
    }
} _cmdDestroy;

class SpiralPatternCommandCreatedEventHandler : public adsk::core::CommandCreatedEventHandler
{
public:
    void notify(const Ptr<CommandCreatedEventArgs>& eventArgs) override
    {
        // Define the command dialog.
        //INPUTS --------------------       
        Ptr<Command> cmd = eventArgs->command();
        cmd->isExecutedWhenPreEmpted(false);
        Ptr<CommandInputs> inputs = cmd->commandInputs();
        if (!checkReturn(inputs))
            return;

        _objectSelection = inputs->addSelectionInput(_commandId + "_objects", "Select Component", "Select bodies or occurrences");
        if (!checkReturn(_objectSelection))
            return;

        _objectSelection->addSelectionFilter("Occurrences");
        _objectSelection->setSelectionLimits(1);

        _axisSelection = inputs->addSelectionInput(_commandId + "_axis", "Select axis", "Select edge or axis of rotation");
        if (!checkReturn(_axisSelection))
            return;

        _axisSelection->addSelectionFilter("CylindricalFaces");
        _axisSelection->addSelectionFilter("LinearEdges");
        _axisSelection->addSelectionFilter("ConstructionLines");
        _axisSelection->addSelectionFilter("CircularEdges");
        _axisSelection->setSelectionLimits(1, 1);

        _number = inputs->addIntegerSliderCommandInput(_commandId + "_number", "Number of Occurrences", 2, 100);
        _height = inputs->addValueInput(_commandId + "_height", "Height Total", "m", adsk::core::ValueInput::createByReal(100));
        _distance = inputs->addValueInput(_commandId + "_distance", "Occurences Distance", "mm", adsk::core::ValueInput::createByReal(25));
        _angle = inputs->addValueInput(_commandId + "_angle", "Occurences Angle", "deg", adsk::core::ValueInput::createByString("20"));

        _distance->value(5);

        // Connect to the command related events.
        Ptr<InputChangedEvent> inputChangedEvent = cmd->inputChanged();
        if (!inputChangedEvent)
            return;

        bool isOk = inputChangedEvent->add(&_cmdInputChanged);
        if (!isOk)
            return;

        Ptr<ValidateInputsEvent> validateInputsEvent = cmd->validateInputs();
        if (!validateInputsEvent)
            return;

        isOk = validateInputsEvent->add(&_cmdValidateInputs);
        if (!isOk)
            return;

        Ptr<CommandEvent> executeEvent = cmd->execute();
        if (!executeEvent)
            return;

        isOk = executeEvent->add(&_spiralPatternCommandExecute);
        if (!isOk)
            return;

        Ptr<CommandEvent> destroyEvent = cmd->destroy();
        if (!destroyEvent)
            return;

        isOk = destroyEvent->add(&_cmdDestroy);
        if (!isOk)
            return;
    }
} _spiralPatternCommandCreated;

extern "C" XI_EXPORT bool run(const char* context)
{
    _app = Application::get();
    if (!_app)
        return false;

    _ui = _app->userInterface();
    if (!_ui)
        return false;

    // Create a command definition and add a button to the CREATE panel.
    Ptr<CommandDefinition> cmdDef = _ui->commandDefinitions()->itemById("SpiralPattern");
    if (!cmdDef)
    {
        cmdDef = _ui->commandDefinitions()->addButtonDefinition("SpiralPattern", "Spiral Pattern", "Creates pattern along spiral path", "");
        if (!checkReturn(cmdDef))
            return false;
    }

    // Connect to the command created event.
    Ptr<CommandCreatedEvent> commandCreatedEvent = cmdDef->commandCreated();
    if (!checkReturn(commandCreatedEvent))
        return false;

    bool isOk = commandCreatedEvent->add(&_spiralPatternCommandCreated);
    if (!isOk)
        return false;

    isOk = cmdDef->execute();
    if (!isOk)
        return false;

    adsk::autoTerminate(false);

    return true;
}

bool is_digits(const std::string& str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

/*
 *  Verfies that a value command input has a valid expression and returns the
 *  value if it does.  Otherwise it returns False.  This works around a
 *  problem where when you get the value from a ValueCommandInput it causes the
 *  current expression to be evaluated and updates the display.  Some new functionality
 *  is being added in the future to the ValueCommandInput object that will make
 *  this easier and should make this function obsolete.
 */
bool getCommandInputValue(Ptr<CommandInput> commandInput, std::string unitType, double* value)
{
    Ptr<ValueCommandInput> valCommandInput = commandInput;
    if (!valCommandInput)
    {
        *value = 0;
        return false;
    }

    // Verify that the expression is valid.
    Ptr<Design> des = _app->activeProduct();
    Ptr<UnitsManager> unitsMgr = des->unitsManager();

    if (unitsMgr->isValidExpression(valCommandInput->expression(), unitType))
    {
        *value = unitsMgr->evaluateExpression(valCommandInput->expression(), unitType);
        return true;
    }
    else
    {
        *value = 0;
        return false;
    }
}

///SELECTION
bool getSelectedComponents(Ptr<SelectionCommandInput> selectionInput) {//Get bodies

    Ptr<Selections> occurs;
    for (int x = 0; x < selectionInput->selectionCount(); ++x) {
        Ptr<Selection> selection = selectionInput->selection(x);
        if (!selection)
            return false;

        Ptr<Occurrence> occur = selection->entity();
        if (!occur)
            return false;

        if ("adsk::fusion::Occurrence" == std::string(occur->objectType())) {
            _occurrences.push_back(occur);
            //_ui->messageBox("Add Occurrence!");
        }
    }
    return true;
}

bool getSelectedAxis(Ptr<SelectionCommandInput>selectionInput) {//Get axis
   //Tuple: 0 = [Point3D - origin], 1 = [Vector3D - direction]
    Ptr<Selection> selection = selectionInput->selection(0);
    if (!selection)
        return false;

    Ptr<Base> selectedObj = selection->entity();
    if (!selectedObj)
        return false;

    //_ui->messageBox(selectedObj->objectType());
    if ("adsk::fusion::BRepEdge" == std::string(selectedObj->objectType()))
    {
        Ptr<BRepEdge> edge = selectedObj;
        if (!edge)
            return false;

        if (std::string(edge->geometry()->objectType()) == "adsk::core::Arc3D") {
            Ptr<Arc3D> arc = edge->geometry();
            if (!arc)
                return false;

            _point = arc->center();
            _normal = arc->normal();
            return true;
        }
        else if (std::string(edge->geometry()->objectType()) == "adsk::core::Circle3D") {//Arc Edge or Circle Edge

            Ptr<Circle3D> circle = edge->geometry();
            if (!circle)
                return false;

            _point = circle->center();
            _normal = circle->normal();
            return true;
        }
        else if (std::string(edge->geometry()->objectType()) == "adsk::core::Line3D") {//Arc Edge or Circle Edge

            Ptr<Circle3D> circle = edge->geometry();
            if (!circle)
                return false;

            _point = circle->center();
            _normal = circle->normal();
            return true;
        }
    }
    else if("adsk::fusion::ConstructionAxis" == std::string(selectedObj->objectType()))
    {
        Ptr<ConstructionAxis> axis = selectedObj;
        if (!axis)
            return false;

        if (axis->geometry()->objectType() == "adsk::core::InfiniteLine3D") { //Construction Axis
            Ptr<InfiniteLine3D> line = axis->geometry();
            if (!line)
                return false;

            _point = line->origin();
            _normal = line->direction();
            return true;
        }
    }
    else if ("adsk::fusion::BRepFace" == std::string(selectedObj->objectType())) {//Cylindrical faces
        Ptr<BRepFace> brep = selectedObj;
        if (!brep)
            return false;

        Ptr<Cylinder> face = brep->geometry();
        if (!face)
            return false;

        _point = face->origin();
        _normal = face->axis();
        return true;
    }

    return false;
}

#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif // XI_WIN
