assert(os.setcwd(package.__dir))
print("current working directory:", os.getcwd())
print("current script file:", package.__chunk, fs.path.stat(package.__chunk))

assert(fs.copyfile(package.__chunk, "✅copied/test.lua"))
assert(fs.copydir("✅copied", "❗party"))
assert(fs.rename("✅copied", "❗party/✨subdir"))

print("file list in ❗party/", fs.enumfiles"❗party")
assert(fs.rmdir"❗party")

-- wait for removal of directory
while fs.path.isdir"❗party" do
    os.sleep(10)
end

assert(#fs.enumfiles"❗party" == 0)
