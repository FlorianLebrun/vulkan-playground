const Path = require("path")
const Process = require("process")
const ChildProcess = require("child_process")
const fs = require("fs")

const command = {
  exec(command, options) {
    const status = ChildProcess.execSync(command, {
      ...options,
      cwd: options && options.cwd && Path.resolve(options.cwd),
      stdio: ['inherit', 'inherit', 'inherit']
    })
    if (status) {
      const message = `Command '${command}' has failed with status code ${status}.`
      if (options && options.ignoreStatus) console.log("[ignored]", message)
      else throw new Error(message)
    }
    return status
  },
  call(program, args, options) {
    const result = ChildProcess.spawnSync(program, args, {
      ...options,
      cwd: options && options.cwd && Path.resolve(options.cwd),
      stdio: ['inherit', 'inherit', 'inherit']
    })
    if (result.error) {
      throw new Error(`Command '${program} ${args ? args.join(" ") : ""}' has crashed: ${result.error}.`)
    }
    else if (result.status) {
      const message = `Command '${program} ${args ? args.join(" ") : ""}' has failed with status code ${result.status}.`
      if (options && options.ignoreStatus) console.log("[ignored]", message)
      else throw new Error(message)
    }
    return result.status
  },
  exit(status) {
    Process.exit(status)
  },
}

const file = {
  exists(path) {
    return fs.existsSync(path) && fs.lstatSync(path).isFile()
  },
  copy: {
    toFile(src, dest) {
      dest = Path.resolve(dest)
      directory.make(Path.dirname(dest))
      fs.copyFileSync(src, dest)
      return dest
    },
    toDir(src, dest) {
      dest = Path.resolve(dest, Path.basename(src))
      directory.make(Path.dirname(dest))
      fs.copyFileSync(src, dest)
      return dest
    }
  },
  move: {
    toFile(src, dest) {
      dest = Path.resolve(dest)
      directory.make(Path.dirname(dest))
      fs.copyFileSync(src, dest)
      fs.unlinkSync(src)
      return dest
    },
    toDir(src, dest) {
      dest = Path.resolve(dest, Path.basename(src))
      directory.make(Path.dirname(dest))
      fs.copyFileSync(src, dest)
      fs.unlinkSync(src)
      return dest
    },
  },
  read: {
    json(path) {
      try { return JSON.parse(fs.readFileSync(path).toString()) }
      catch (e) { return undefined }
    },
    text(path) {
      try { return fs.readFileSync(path).toString() }
      catch (e) { return undefined }
    }
  },
  write: {
    json(path, data) {
      directory.make(Path.dirname(path))
      fs.writeFileSync(path, JSON.stringify(data, null, 2))
    },
    text(path, data) {
      directory.make(Path.dirname(path))
      fs.writeFileSync(path, Array.isArray(data) ? data.join("\n") : data.toString())
    }
  },
  remove(path) {
    if (fs.existsSync(path)) {
      fs.unlinkSync(path)
    }
  },
}

const directory = {
  exists(path) {
    return fs.existsSync(path) && fs.lstatSync(path).isDirectory()
  },
  filenames(path) {
    try { return fs.readdirSync(path) || [] }
    catch (e) { return [] }
  },
  copy(src, dest, filter) {
    if (directory.exists(src)) {
      for (const name of fs.readdirSync(src)) {
        const path = Path.join(src, name)
        const stats = fs.lstatSync(path)
        const destination = Path.join(dest, name)
        if (stats.isDirectory()) {
          directory.copy(path, destination, filter)
        }
        else if (!filter || filter(name, path, stats)) {
          file.copy.toFile(path, destination)
        }
      }
    }
  },
  make(path) {
    if (path && !fs.existsSync(path)) {
      directory.make(Path.parse(path).dir)
      fs.mkdirSync(path)
    }
  },
  remove(path) {
    if (fs.existsSync(path) && fs.lstatSync(path).isDirectory()) {
      fs.readdirSync(path).forEach(function (entry) {
        var entry_path = Path.join(path, entry)
        if (fs.lstatSync(entry_path).isDirectory()) {
          directory.remove(entry_path)
        }
        else {
          try { file.remove(entry_path) }
          catch (e) { return }
        }
      })
      fs.rmdirSync(path)
    }
  },
  clean(path) {
    directory.remove(path)
    directory.make(path)
  },
}

const colors = {
  Reset: "\x1b[0m",
  Bright: "\x1b[1m",
  Dim: "\x1b[2m",
  Underscore: "\x1b[4m",
  Blink: "\x1b[5m",
  Reverse: "\x1b[7m",
  Hidden: "\x1b[8m",
  Black: "\x1b[30m",
  Red: "\x1b[31m",
  Green: "\x1b[32m",
  Yellow: "\x1b[33m",
  Blue: "\x1b[34m",
  Magenta: "\x1b[35m",
  Cyan: "\x1b[36m",
  White: "\x1b[37m",
  BgBlack: "\x1b[40m",
  BgRed: "\x1b[41m",
  BgGreen: "\x1b[42m",
  BgYellow: "\x1b[43m",
  BgBlue: "\x1b[44m",
  BgMagenta: "\x1b[45m",
  BgCyan: "\x1b[46m",
  BgWhite: "\x1b[47m",
};
function printColored(colorTag) {
  return function (...args) {
    console.log(colorTag, ...args, colors.Reset);
  }
}
const print = {
  log: printColored(colors.White),
  debug: printColored(colors.Magenta),
  warning: printColored(colors.Magenta),
  error: printColored(colors.Bright + colors.Red),
  success: printColored(colors.Green),
  title: printColored(colors.Cyan),
  info: printColored(colors.Yellow),
  exception: function (e) {
    print.error(e.message);
    printColored(colors.Red)(e.stack);
  },
}

function script(callback, descriptor) {
  try {
    const argv = {}
    const argvInfos = descriptor && descriptor.arguments

    // Read arguments collection
    let key = "0"
    for (let i = 0; i < Process.argv.length; i++) {
      const value = Process.argv[i]
      if (value.startsWith("--")) {
        key = value.substr(2)
        if (argvInfos) {
          const schema = argvInfos[key]
          if (!schema) {
            throw new Error(`argument '${key}' invalid: is not a recognized input`)
          }
          if (schema.closure === true) {
            argv[key] = Process.argv.slice(i + 1)
            break
          }
        }
        argv[key] = true
      }
      else {
        if (Array.isArray(argv[key])) argv[key].push(value)
        else if (argv[key] === true) argv[key] = value
        else argv[key] = [argv[key], value]
      }
    }

    // Check arguments collection
    for (const key in argvInfos) {
      const schema = argvInfos[key]
      let value = argv[key]
      try {
        if (value !== undefined) {
          const checkScalar = (value) => {
            if (Array.isArray(schema.enums) && schema.enums.indexOf(value) < 0) {
              throw new Error(`value '${value}' is not in [${schema.enums.join(", ")}]`)
            }
            if (schema.type === "string") {
              if (value === true) value = ""
              if (typeof value !== "string") throw new Error(`shall be a string`)
            }
            else if (schema.type === "number") {
              value = parseFloat(value)
            }
            else if (schema.type === "object") {
              value = JSON.parse(value)
            }
            else if (schema.type === "boolean") {
              if (typeof value === "string") {
                value = value.toLowerCase()
                if (value === "true" || value === "on" || value === "1") value = true
                else if (value === "false" || value === "off" || value === "0") value = false
                else throw new Error(`shall string convertable into boolean`)
              }
              if (typeof value !== "boolean") {
                throw new Error(`shall be a boolean`)
              }
            }
            return value
          }
          if (Array.isArray(value)) {
            value = value.map(checkScalar)
            if (schema.isArray !== true) throw new Error("cannot be an array")
          }
          else {
            value = checkScalar(value)
            if (schema.isArray === true) value = [value]
          }
        }
        else {
          if (schema.required === true) throw new Error(`is required`)
          else if (schema.default !== undefined) value = schema.default
          else if (schema.type === "string") value = ""
          else if (schema.type === "number") value = 0
          else if (schema.type === "object") value = {}
          else if (schema.type === "boolean") value = false
        }
        argv[key] = value
      }
      catch (e) {
        e.message = `argument '${key}' invalid: ${e.message}`
        throw e
      }
    }

    // Execute script
    callback(argv)
  }
  catch (e) {
    //print.error(e.message)
    print.exception(e)
    print.error(" >>> Failed script:", Process.argv.join(" "))
    Process.exit(1)
  }
}

module.exports = { script, print, command, file, directory }
