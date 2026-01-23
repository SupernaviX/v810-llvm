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
exports.activate = exports.LLDBDapExtension = void 0;
const vscode = require("vscode");
const disposable_context_1 = require("./disposable-context");
const debug_adapter_factory_1 = require("./debug-adapter-factory");
/**
 * This creates the configurations for this project if used as a standalone
 * extension.
 */
function createDefaultLLDBDapOptions() {
    return {
        debuggerType: "lldb-dap",
        createDapExecutableCommand(session, packageJSONExecutable) {
            var _a;
            return __awaiter(this, void 0, void 0, function* () {
                const config = vscode.workspace.getConfiguration("lldb-dap", session.workspaceFolder);
                const path = config.get("executable-path");
                const log_path = config.get("log-path");
                let env = {};
                if (log_path) {
                    env["LLDBDAP_LOG"] = log_path;
                }
                const configEnvironment = config.get("environment") || {};
                if (path) {
                    const dbgOptions = {
                        env: Object.assign(Object.assign({}, configEnvironment), env)
                    };
                    return new vscode.DebugAdapterExecutable(path, [], dbgOptions);
                }
                else if (packageJSONExecutable) {
                    return new vscode.DebugAdapterExecutable(packageJSONExecutable.command, packageJSONExecutable.args, Object.assign(Object.assign({}, packageJSONExecutable.options), { env: Object.assign(Object.assign(Object.assign({}, (_a = packageJSONExecutable.options) === null || _a === void 0 ? void 0 : _a.env), configEnvironment), env) }));
                }
                else {
                    return undefined;
                }
            });
        },
    };
}
/**
 * This class represents the extension and manages its life cycle. Other extensions
 * using it as as library should use this class as the main entry point.
 */
class LLDBDapExtension extends disposable_context_1.DisposableContext {
    constructor(lldbDapOptions) {
        super();
        this.lldbDapOptions = lldbDapOptions;
        this.pushSubscription(vscode.debug.registerDebugAdapterDescriptorFactory(this.lldbDapOptions.debuggerType, new debug_adapter_factory_1.LLDBDapDescriptorFactory(this.lldbDapOptions)));
        this.pushSubscription(vscode.workspace.onDidChangeConfiguration((event) => __awaiter(this, void 0, void 0, function* () {
            if (event.affectsConfiguration("lldb-dap.executable-path")) {
                const dapPath = vscode.workspace
                    .getConfiguration("lldb-dap")
                    .get("executable-path");
                if (dapPath) {
                    const fileUri = vscode.Uri.file(dapPath);
                    if (yield debug_adapter_factory_1.LLDBDapDescriptorFactory.isValidDebugAdapterPath(fileUri)) {
                        return;
                    }
                }
                debug_adapter_factory_1.LLDBDapDescriptorFactory.showLLDBDapNotFoundMessage(dapPath || "");
            }
        })));
    }
}
exports.LLDBDapExtension = LLDBDapExtension;
/**
 * This is the entry point when initialized by VS Code.
 */
function activate(context) {
    context.subscriptions.push(new LLDBDapExtension(createDefaultLLDBDapOptions()));
}
exports.activate = activate;
//# sourceMappingURL=extension.js.map