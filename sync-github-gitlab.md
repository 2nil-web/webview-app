<h3><u><center>Adding gitlab to this github repo</center></u></h3>

#### On gitlab

Created a repo named webview-app (not sure that name is important).
Added branch master as default (setting >> repository >> Branch defaults).
Added branch master as Protected branch.
Allowed to push and merge for Maintainers.
Cloned gitlab repo and deleted main branch locally and remotely : 

```bash
git clone https://gitlab.com/dplalanne/webview-app.git
cd webview-app
git branch -d main
git push origin --delete main
```

#### On github local repo

```bash
cd ../2-webview-app
git remote set-url --add --push origin https://github.com/2nil-web/webview-app.git
git remote add gitlab https://gitlab.com/dplalanne/webview-app.git
git remote set-url --add --push origin https://gitlab.com/dplalanne/webview-app.git
git add sync-github-gitlab.md
gitci (git commit -a -m "..." ; git push)
git branch -r
  gitlab/master
  origin/HEAD -> origin/master
  origin/master`
```

#### Conclusion

- Checked on both site that file 'sync-github-gitlab.md' has been added.
- Deleted gitlab local repo.
  ```bash
  cd ..
  rm -rf webview-app
- Now any git push will be done on both remote repositories (I hope).
