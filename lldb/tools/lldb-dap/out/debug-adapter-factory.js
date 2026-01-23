"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.LLDBDapDescriptorFactory = void 0;
const vscode = require("vscode");
/**
 * This class defines a factory used to find the lldb-dap binary to use
 * depending on the session configuration.
 */
class LLDBDapDescriptorFactory {
    constructor(lldbDapOptions) {
        this.lldbDapOptions = lldbDapOptions;
    }
    static isValidDebugAdapterPath(pathUri) {
        return __awaiter(this, void 0, void 0, function* () {
            try {
                const fileStats = yield vscode.workspace.fs.stat(pathUri);
                if (!(fileStats.type & vscode.FileType.File)) {
                    return false;
                }
            }
            catch (err) {
                return false;
            }
            return true;
        });
    }
    createDebugAdapterDescriptor(session, executable) {
        return __awaiter(this, void 0, void 0, function* () {
            const config = vscode.workspace.getConfiguration("lldb-dap", session.workspaceFolder);
            const customPath = config.get("executable-path");
            const path = customPath || executable.command;
            const fileUri = vscode.Uri.file(path);
            if (!(yield LLDBDapDescriptorFactory.isValidDebugAdapterPath(fileUri))) {
                LLDBDapDescriptorFactory.showLLDBDapNotFoundMessage(fileUri.path);
            }
            return this.lldbDapOptions.createDapExecutableCommand(session, executable);
        });
    }
    /**
     * Shows a message box when the debug adapter's path is not found
     */
    static showLLDBDapNotFoundMessage(path) {
        return __awaiter(this, void 0, void 0, function* () {
            const openSettingsAction = "Open Settings";
            const callbackValue = yield vscode.window.showErrorMessage(`Debug adapter path: ${path} is not a valid file`, openSettingsAction);
            if (openSettingsAction === callbackValue) {
                vscode.commands.executeCommand("workbench.action.openSettings", "lldb-dap.executable-path");
            }
        });
    }
}
exports.LLDBDapDescriptorFactory = LLDBDapDescriptorFactory;
//# sourceMappingURL=debug-adapter-factory.js.map